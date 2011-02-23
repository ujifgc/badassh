#include <ctype.h>
#include <stdlib.h>
#include <assert.h>

#define DEFINE_PLUG_METHOD_MACROS

extern "C" {
#include "putty.h"
}

#include "Form1.h"
#include "badanet.h"

extern Form1 *mainForm;
extern Terminal *term;
extern badaNet *skynet;

using namespace Osp::Base;
using namespace Osp::Ui::Controls;
using namespace Osp::Base::Collection;
using namespace Osp::Base::Utility;
using namespace Osp::Net;
using namespace Osp::Base::Runtime;
using namespace Osp::Net::Sockets;
using namespace Osp::Net::Wifi;

#include "sk_tcp.h"

result badaNet::ConstructConnection(bool do_activate) {
	result r = E_SUCCESS;
	if (!pWifiMgr) {
		pWifiMgr = new WifiManager();
		pWifiMgr->Construct(*this);
	}

	if (pWifiMgr->IsActivated()) {
		r = ConstructAccount();
	}else if (do_activate) {
		manualWiFi = true;
		mainForm->SetStatus("Starting Wi-Fi");
		r = pWifiMgr->Activate();
	}
	return r;
}

result badaNet::ConstructAccount() {
	result r = E_FAILURE;
	active = false;
	NetAccountManager accountManager;
	accountManager.Construct();
	NetAccountId accountId = accountManager.GetNetAccountId(NET_BEARER_WIFI);
	if (accountId == -1)
		return E_FAILURE;

	if (!pConn) {
		pConn = new NetConnection();
		if (E_SUCCESS != pConn->Construct(accountId))
			return E_FAILURE;
		r = pConn->AddNetConnectionListener(*this);
	}else{
		r = E_SUCCESS;
	}
	if (E_SUCCESS == r) {
		active = true;
		if (!start_on_activate)
			mainForm->ConnectionActive();
	}
	return r;
}

badaNet::badaNet() {
	active = false;
	pConn = 0;
	pDns = 0;
	addr = 0;
	manualWiFi = false;
	start_on_activate = false;
}

badaNet::~badaNet() {
	if (pConn)
		delete pConn;
	if (pDns)
		delete pDns;
	if (addr)
		sk_addr_free(addr);

	if (pWifiMgr) {
		if (manualWiFi && pWifiMgr->IsConnected())
			pWifiMgr->Deactivate();
		delete pWifiMgr;
	}
}

void badaNet::OnDnsResolutionCompletedN(IpHostEntry* ipHostEntry, result r) {
	if (r == E_SUCCESS && ipHostEntry != null) {
		IList* addressList = ipHostEntry->GetAddressList();
		Ip4Address* pIp4Address = static_cast<Ip4Address*> (addressList->GetAt(0));
		addr->pHostaddr = new String(pIp4Address->ToString());
		char buf[64] = { 0 };
		wcstombs(buf, pIp4Address->ToString().GetPointer(), 64);
		delete ipHostEntry;
		/*
		 * go to ssh backend, let her know we're done
		 */
		plug_dns(plug, addr);
	}else{
		plug_dns(plug, NULL);
	}
	delete pDns;
	pDns = 0;
}

void badaNet::OnNetConnectionStarted(NetConnection& netConnection, result r) {
	if (!pDns)
		pDns = new Dns();
	r = pDns->Construct(*this);
	mainForm->SetStatus("Resolving");
	if (addr->pHostname->GetLength() > 0)
		r = pDns->GetHostByName(*addr->pHostname);
	else
		r = E_INVALID_ARG;
	if (r != E_SUCCESS)
		OnDnsResolutionCompletedN(NULL, r);
	/*
	 * liberate the thread and let the dns listener talk to ssh backend
	 */
}

void badaNet::OnNetConnectionStopped(NetConnection& netConnection, result r) {
	mainForm->ConnectionStopped();
	active = false;
	CloseSocket();
	if (pConn) {
		delete pConn;
		pConn = 0;
	}
}

void badaNet::OnNetConnectionSuspended(Osp::Net::NetConnection& netConnection) {
	if (sock)
		sk_tcp_set_frozen((::Socket)sock, 1);
}
void badaNet::OnNetConnectionResumed(Osp::Net::NetConnection& netConnection) {
	if (sock)
		sk_tcp_set_frozen((::Socket)sock, 0);
}

void badaNet::OnSocketAccept(Osp::Net::Sockets::Socket &socket) {
	Actual_Socket s = sock;

	Osp::Net::Sockets::Socket *newsock = socket.AcceptN();
	if (!newsock)
		return;
	if (plug_accepting(s->plug, (void*) newsock))
		newsock->Close();
}

void badaNet::OnSocketClosed(Osp::Net::Sockets::Socket &socket, NetSocketClosedReason reason) {
	Actual_Socket s = sock;
	char buf[20480];
	int open = 1, ret;
	result r;
	do {
		r = socket.Receive(buf, sizeof(buf), ret);
		if (ret < 0) {
			if (r == E_WOULD_BLOCK)
				break;
			plug_closing(s->plug, "Host does not respond\n", r, 0);
			return;
		} else {
			if (ret)
				open &= plug_receive(s->plug, 0, buf, ret);
			else
				open &= plug_closing(s->plug, NULL, 0, 0);
		}
	} while (ret > 0);
}

void badaNet::OnSocketConnected(Osp::Net::Sockets::Socket &socket) {
	Actual_Socket s = sock;

	s->connected = s->writable = 1;
	mainForm->SocketConnected();
}

void badaNet::OnSocketReadyToReceive(Osp::Net::Sockets::Socket &socket) {
	Actual_Socket s = sock;

	if (s->frozen) {
		s->frozen_readable = 1;
		return;
	}

	char buf[20480];
	int ret = 0;

	mainForm->SetStatus("Waiting");
	result r = socket.Receive(buf, sizeof(buf), ret);
	noise_ultralight(ret);
	if (ret < 0 && r == E_WOULD_BLOCK)
		return;
	if (ret < 0) {
		plug_closing(s->plug, "Infernal error\n", r, 0);
		return;
	} else if (0 == ret) {
		plug_closing(s->plug, NULL, 0, 0);
		return;
	} else {
		plug_receive(s->plug, 0, buf, ret);
		return;
	}
}
void badaNet::OnSocketReadyToSend(Osp::Net::Sockets::Socket &socket) {
	Actual_Socket s = sock;
	result r = E_SUCCESS;

	int bufsize_before, bufsize_after;
	s->writable = 1;
	bufsize_before = s->sending_oob + bufchain_size(&s->output_data);
	r = TrySend(s);
	bufsize_after = s->sending_oob + bufchain_size(&s->output_data);
	if (bufsize_after < bufsize_before)
		plug_sent(s->plug, bufsize_after);
}

void badaNet::OnWifiConnected(const Osp::Base::String& ssid, result r) {
	if (E_SUCCESS == r) {
		ConstructAccount();
		if (start_on_activate)
			r = StartConnection(true);
	}
}

result badaNet::DnsRequest(String hostName, Plug p) {
	plug = p;

	result r = E_SUCCESS;

	if (addr) sk_addr_free(addr);
	addr = snew(struct SockAddr_tag);
	addr->pHostname = new String(hostName);
	addr->pHostaddr = NULL;

	if (!active) {
		start_on_activate = true;
		return E_SUCCESS;
	}

	r = StartConnection(true);
	/*
	 * let the listener resolve
	 */
	return r;
}

bool badaNet::IsActive() {
	return active;
}

result badaNet::StartConnection(bool wait) {
	if (!active)
		return E_FAILURE;
	state = pConn->GetConnectionState();
	switch (state) {
	case NET_CONNECTION_STATE_STARTED:
	case NET_CONNECTION_STATE_RESUMED: // if connection is started, fire the start event
		OnNetConnectionStarted(*pConn, E_SUCCESS);
		return E_SUCCESS;
	default:                           // return success and let the listener listen for the start event
		mainForm->SetStatus("Connecting");
		return pConn->Start();
	}

	return E_FAILURE;
}

result badaNet::CloseSocket() {
	result r = E_SUCCESS;
	if (pSocket) {
		pSocket->AsyncSelectByListener(0);
		r = pSocket->Close();
		delete pSocket;
		pSocket = 0;
	}
	return r;
}

result badaNet::TryConnect(Actual_Socket as) {
	sock = as;
	result r = E_SUCCESS;

	if (pSocket) {
		r = pSocket->AsyncSelectByListener(0);
		r = pSocket->Close();
		delete pSocket;
		pSocket = 0;
	}

	pSocket = new Osp::Net::Sockets::Socket();
	r = pSocket->Construct(NET_SOCKET_AF_IPV4, NET_SOCKET_TYPE_STREAM, NET_SOCKET_PROTOCOL_TCP);
	if (as->oobinline)
		r = pSocket->SetSockOpt(NET_SOCKET_SOL_SOCKET, NET_SOCKET_SO_OOBINLINE, 1);
	if (as->nodelay)
		r = pSocket->SetSockOpt(NET_SOCKET_SOL_SOCKET, NET_SOCKET_TCP_NODELAY, 1);
	if (as->keepalive)
		r = pSocket->SetSockOpt(NET_SOCKET_SOL_SOCKET, NET_SOCKET_SO_KEEPALIVE, 1);
	r = pSocket->AddSocketListener(*this);
	DoSelect(1);

	Ip4Address peerAddr(*addr->pHostaddr);
	unsigned short peerPort = as->port;
	NetEndPoint peerEndPoint(peerAddr, peerPort);
	mainForm->SetStatus("Connecting");
	r = pSocket->Connect(peerEndPoint);
	switch (r) {
	case E_SUCCESS:
		as->writable = 1;
		break;
	case E_WOULD_BLOCK:
		break;
	default:
		notify_remote_exit(0);
		return r;
	}
	return E_SUCCESS;
}

result badaNet::DoSelect(int startup) {
	result r = E_SUCCESS;
	if (!pSocket)
		return E_INVALID_SOCKET;

	r = pSocket->AsyncSelectByListener(startup ? (NET_SOCKET_EVENT_CONNECT | NET_SOCKET_EVENT_READ
			| NET_SOCKET_EVENT_WRITE | NET_SOCKET_EVENT_CLOSE | NET_SOCKET_EVENT_ACCEPT
			| NET_SOCKET_EVENT_SERVCERT_FAIL) : 0);
	return r;
}

result badaNet::TrySend(Actual_Socket s) {
	sock = s;
	result r = E_SUCCESS;
	if (!pSocket)
		return E_INVALID_SOCKET;
	while (s->sending_oob || bufchain_size(&s->output_data) > 0) {
		int nsent;
		void *data;
		int len;

		if (s->sending_oob) {
			len = s->sending_oob;
			data = &s->oobdata;
		} else {
			bufchain_prefix(&s->output_data, &data, &len);
		}
		mainForm->SetStatus("Talking to host...");
		r = pSocket->Send(data, len, nsent);
		noise_ultralight(nsent);
		if (nsent <= 0) {
			if (r == E_WOULD_BLOCK) {
				s->writable = FALSE;
				return r;
			} else {
				s->pending_error = r;
				mainForm->SetStatus("Disconnected");
				return r;
			}
		} else {
			if (s->sending_oob) {
				if (nsent < len) {
					bada_memmove(s->oobdata, s->oobdata + nsent, len - nsent);
					s->sending_oob = len - nsent;
				} else {
					s->sending_oob = 0;
				}
			} else {
				bufchain_consume(&s->output_data, nsent);
			}
		}
	}
	return r;
}

void connection_fatal(void *frontend, char *p, ...) {
	va_list ap;
	va_start(ap, p);
	char *msg = dupvprintf(p, ap);
	va_end(ap);
	AppLog(msg);
	if (term)
		term_data_untrusted(term, msg, strlen(msg));
	sfree(msg);
	notify_remote_exit(frontend);
}

void logevent(void *frontend, const char *string) {
#ifdef _DEBUG
	AppLog(string);
#endif
}

void name_lookup(const char *host, Plug plug) {
	if (!skynet)
		return;
	skynet->DnsRequest(host, plug);
}

void sk_getaddr(SockAddr addr, char *buf, int buflen) {
	if (addr->pHostaddr)
		wcstombs(buf, addr->pHostaddr->GetPointer(), buflen);
	else
		buf[0] = 0;
}

const char *sk_addr_error(SockAddr addr) {
	return NULL; //TODO
}

void sk_addr_free(SockAddr addr) {
	if (!addr)
		return;
	if (addr->pHostaddr)
		delete addr->pHostaddr;
	if (addr->pHostname)
		delete addr->pHostname;
	sfree(addr);
}

static Plug sk_tcp_plug(::Socket sock, Plug p) {
	Actual_Socket s = (Actual_Socket) sock;
	Plug ret = s->plug;
	if (p)
		s->plug = p;
	return ret;
}

static void sk_tcp_flush(::Socket s) {
}

static void sk_tcp_close(::Socket sock) {
	Actual_Socket s = (Actual_Socket) sock;

	if (s->child)
		sk_tcp_close((::Socket) s->child);

	skynet->DoSelect(0);
	skynet->CloseSocket();

	sfree(s);
}

static int sk_tcp_write(::Socket sock, const char *buf, int len) {
	Actual_Socket s = (Actual_Socket) sock;

	bufchain_add(&s->output_data, buf, len);

	if (s->writable)
		skynet->TrySend(s);

	return bufchain_size(&s->output_data);
}

static int sk_tcp_write_oob(::Socket sock, const char *buf, int len) {
	Actual_Socket s = (Actual_Socket) sock;

	bufchain_clear(&s->output_data);
	bada_memcpy(s->oobdata, buf, len);
	s->sending_oob = len;

	if (s->writable)
		skynet->TrySend(s);

	return s->sending_oob;
}

static void sk_tcp_set_private_ptr(::Socket sock, void *ptr) {
	Actual_Socket s = (Actual_Socket) sock;
	s->private_ptr = ptr;
}

static void *sk_tcp_get_private_ptr(::Socket sock) {
	Actual_Socket s = (Actual_Socket) sock;
	return s->private_ptr;
}

static const char *sk_tcp_socket_error(::Socket sock) {
	Actual_Socket s = (Actual_Socket) sock;
	return s->error;
}

::Socket sk_new(SockAddr addr, int port, int privport, int oobinline, int nodelay, int keepalive, Plug plug) {
	static const struct socket_function_table fn_table = { sk_tcp_plug, sk_tcp_close, sk_tcp_write, sk_tcp_write_oob,
			sk_tcp_flush, sk_tcp_set_private_ptr, sk_tcp_get_private_ptr, sk_tcp_set_frozen, sk_tcp_socket_error };

	Actual_Socket ret;

	if (!skynet)
		return 0;

	/*
	 * Create Socket structure.
	 */
	ret = snew(struct Socket_tag);
	ret->fn = &fn_table;
	ret->error = NULL;
	ret->plug = plug;
	bufchain_init(&ret->output_data);
	ret->connected = 0; /* to start with */
	ret->writable = 0; /* to start with */
	ret->sending_oob = 0;
	ret->frozen = 0;
	ret->frozen_readable = 0;
	ret->localhost_only = 0; /* unused, but best init anyway */
	ret->pending_error = 0;
	ret->parent = ret->child = NULL;
	ret->oobinline = oobinline;
	ret->nodelay = nodelay;
	ret->keepalive = keepalive;
	ret->privport = privport;
	ret->port = port;
	ret->s = NULL;

	skynet->TryConnect(ret);

	return (::Socket) ret;
}

::Socket new_connection(SockAddr addr, char *hostname, int port, int privport, int oobinline, int nodelay,
		int keepalive, Plug plug, const Config *cfg) {
	return sk_new(addr, port, privport, oobinline, nodelay, keepalive, plug);
}
