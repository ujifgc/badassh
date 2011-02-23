#ifndef BADANET_H
#define BADANET_H

#include <FBase.h>
#include <FNet.h>
#include <FNetIPAddress.h>
#include <FNetIP4Address.h>
#include <FNetSockSocket.h>
#include <FBaseRtThreadMutex.h>
#include <FBaseRtThreadThread.h>
#include <FNetWifiIWifiManagerEventListener.h>

struct SockAddr_tag {
	Osp::Net::NetAddressFamily family;
	Osp::Base::String *pHostname;
	Osp::Base::String *pHostaddr;
};

typedef struct Socket_tag *Actual_Socket;

struct Socket_tag {
	const struct socket_function_table *fn;
	/* the above variable absolutely *must* be the first in this structure */
	char *error;
	Osp::Net::Sockets::Socket *s;
	Plug plug;
	void *private_ptr;
	bufchain output_data;
	int connected;
	int writable;
	int frozen; /* this causes readability notifications to be ignored */
	int frozen_readable; /* this means we missed at least one readability
	 * notification while we were frozen */
	int localhost_only; /* for listening sockets */
	char oobdata[1];
	int sending_oob;
	int oobinline, nodelay, keepalive, privport;
	//SockAddr addr;
	int port;
	int pending_error; /* in case send() returns error */
	/*
	 * We sometimes need pairs of Socket structures to be linked:
	 * if we are listening on the same IPv6 and v4 port, for
	 * example. So here we define `parent' and `child' pointers to
	 * track this link.
	 */
	Actual_Socket parent, child;
};

class badaNet:
	public Osp::Base::Object,
	public Osp::Net::IDnsEventListener,
	public Osp::Net::INetConnectionEventListener,
	public Osp::Net::Sockets::ISocketEventListener,
	public Osp::Net::Wifi::IWifiManagerEventListener {
protected:
	Osp::Net::Dns *pDns;
	Osp::Net::NetConnectionState state;
	bool active, manualWiFi, start_on_activate;
	Osp::Net::NetConnection *pConn;
	Osp::Net::Sockets::Socket *pSocket;
	Osp::Net::Wifi::WifiManager *pWifiMgr;
	Actual_Socket sock;
	SockAddr addr;
	Plug plug;
	//Dns
	void OnDnsResolutionCompletedN(Osp::Net::IpHostEntry* ipHostEntry, result r);
	//NetConnection
	void OnNetConnectionStarted(Osp::Net::NetConnection& netConnection, result r);
	void OnNetConnectionStopped(Osp::Net::NetConnection& netConnection, result r);
	void OnNetConnectionSuspended(Osp::Net::NetConnection& netConnection);
	void OnNetConnectionResumed(Osp::Net::NetConnection& netConnection);
	//Socket
	void OnSocketAccept(Osp::Net::Sockets::Socket &socket);
	void OnSocketClosed(Osp::Net::Sockets::Socket &socket, Osp::Net::Sockets::NetSocketClosedReason reason);
	void OnSocketConnected(Osp::Net::Sockets::Socket &socket);
	void OnSocketReadyToReceive(Osp::Net::Sockets::Socket &socket);
	void OnSocketReadyToSend(Osp::Net::Sockets::Socket &socket);
	//WiFi
	void OnWifiActivated(result r) {}
	void OnWifiDeactivated(result r) {}
	void OnWifiConnected(const Osp::Base::String& ssid, result r);
	void OnWifiDisconnected(void) {}
	void OnWifiRssiChanged(long rssi) {}
	void OnWifiScanCompletedN(const Osp::Base::Collection::IList *pWifiBssInfoList, result r) {}
public:
	badaNet(void);
	~badaNet(void);
	result ConstructConnection(bool);
	result ConstructAccount();

	result StartConnection(bool);
	result DnsRequest(Osp::Base::String hostName, Plug plug);
	result TryConnect(Actual_Socket as);
	result TrySend(Actual_Socket s);
	result DoSelect(int startup);
	result CloseSocket();
	bool IsActive();
};

#endif
