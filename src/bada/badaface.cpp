extern "C" {
#include "putty.h"
#include "storage.h"
}

#include <FBase.h>
#include <FUiCtrlMessageBox.h>

using namespace Osp::Ui::Controls;
using namespace Osp::Base;

static int busy_status = BUSY_NOT;

int verify_ssh_host_key(void *frontend, char *host, int port, char *keytype, char *keystr, char *fingerprint,
		void(*callback)(void *ctx, int result), void *ctx) {
	static const char wrongmsg[] = "POTENTIAL SECURITY BREACH!\n"
		"The server's host key changed!\n"
		"The new %s key is: %s\n"
		"Yes - update and go on.\n"
		"No - no update and go on.\n"
		"Cancel - get out.";

	static const char absentmsg[] = "Save server's host key?\n"
		"The server's %s key is: %s\n"
		"Yes - update and go on.\n"
		"No - no update and go on.\n"
		"Cancel - get out.";

	static const char mbtitle[] = "SSH Security Alert";

	const char *msg = 0;

	switch (verify_host_key(host, port, keytype, keystr)) {
	case 0: /* success - key matched OK */
		return 1;
	case 1: /* key was absent */
		msg = absentmsg;
		break;
	case 2: /* key was different */
		msg = wrongmsg;
		break;
	default:
		return 0;
	}

	char *text = dupprintf(msg, keytype, fingerprint);
	char *caption = dupprintf(mbtitle, appname);
	MessageBox *box = new MessageBox;
	box->Construct(String(caption), String(text), MSGBOX_STYLE_YESNOCANCEL);
	sfree(text);
	sfree(caption);

	int ret;
	box->ShowAndWait(ret);
	delete box;

	switch (ret) {
	case MSGBOX_RESULT_YES:
		store_host_key(host, port, keytype, keystr);
		return 1;
	case MSGBOX_RESULT_NO:
		return 1;
	default:
		return 0;
	}
}

void set_busy_status(void *frontend, int status) {
	busy_status = status;
}

int askalg(void *frontend, const char *algtype, const char *algname, void(*callback)(void *ctx, int result), void *ctx) {
	return 0;
}

void modalfatalbox(char *format, ...) {
	__builtin_va_list list;
	__builtin_va_start(list, format);
	char buf[399];
	__builtin_vsnprintf(buf, 399, format, list);
	__builtin_va_end(list);
	MessageBox *box = new MessageBox;
	box->Construct(String("Infetnal error"), String(buf), MSGBOX_STYLE_OK);
	int ret;
	box->ShowAndWait(ret);
	delete box;
}

void fatalbox(char *format, ...) {
	__builtin_va_list list;
	__builtin_va_start(list, format);
	char buf[399];
	__builtin_vsnprintf(buf, 399, format, list);
	__builtin_va_end(list);
	MessageBox *box = new MessageBox;
	box->Construct(String("Infetnal error"), String(buf), MSGBOX_STYLE_OK);
	int ret;
	box->ShowAndWait(ret);
	delete box;
}
