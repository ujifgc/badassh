extern "C" {
#include "putty.h"
#include "terminal.h"
}

#include <stdlib.h>
#include "Form1.h"
#include "Form2.h"
#include "badanet.h"

using namespace Osp::Base;
using namespace Osp::App;
using namespace Osp::Ui;
using namespace Osp::Io;
using namespace Osp::Ui::Controls;
using namespace Osp::Base::Runtime;
using namespace Osp::Graphics;

int default_protocol = 0;
int default_port = 0;
int offset_width = 0, offset_height = 0;
int font_width, font_height, font_dualwidth = 0;
int screen_width = 480, screen_height = 800;

unicode_data ucsdata;
void init_fonts(int, int);
extern "C" void logerror(const char *format, ...);

Config cfg;
Terminal *term = 0;
Backend *back = 0;
extern Form1 *mainForm;
extern Form2 *optionsForm;
extern Frame *mainFrame;
static long timing_next_time;
static void *ldisc;
static void *backhandle;
badaNet *skynet = 0;

Form1::Form1() {
}

Form1::~Form1() {
}

bool Form1::Initialize() {
	default_protocol = be_default_protocol;
	for (int i = 0; backends[i].backend != NULL; i++)
		if (backends[i].protocol == default_protocol) {
			default_port = backends[i].backend->default_port;
			break;
		}

	font_width = 12;
	font_height = 20;
	update = false;
	started = false;
	typing = false;
	pShadow = 0;

	// Construct an XML form
	Construct(L"IDF_FORM1");

	return true;
}

result Form1::OnInitializing(void) {
	SetSoftkeyActionId(SOFTKEY_0, ID_SOFTKEY0);
	//SetSoftkeyActionId(SOFTKEY_1, ID_SOFTKEY1);
	SetOptionkeyActionId(ID_OPTIONKEY);
	AddSoftkeyActionListener(SOFTKEY_0, *this);
	//AddSoftkeyActionListener(SOFTKEY_1, *this);
	AddOptionkeyActionListener(*this);

	AddKeyEventListener(*this);
	AddOrientationEventListener(*this);

	pScroll = static_cast<ScrollPanel *> (GetControl(L"IDC_SCROLLPANEL1"));
	pScroll->AddTouchEventListener(*this);

	pEdit = static_cast<EditArea *>(GetControl(L"IDPC_EDITAREA1", true));
	pEdit->SetLowerCaseModeEnabled(true);
	pEdit->AddTextEventListener(*this);
	pEdit->AddScrollPanelEventListener(*this);
	pEdit->AddActionEventListener(*this);
	pEdit->SetOverlayKeypadCommandButton(COMMAND_BUTTON_POSITION_LEFT, L"Tab", ID_BUTTON_EDITFIELD_DONE);
	pEdit->SetOverlayKeypadCommandButton(COMMAND_BUTTON_POSITION_RIGHT, L"Hide", ID_BUTTON_EDITFIELD_CANCEL);
	pEdit->SetText("+");
	pEdit->SetPosition(800,-120);

	pOptions = new OptionMenu();
	pOptions->Construct();
	pOptions->AddItem("Options", ID_OPTIONMENU_OPTIONS);
	pOptions->AddItem("Disconnect", ID_OPTIONMENU_DISCONNECT);
	pOptions->AddActionEventListener(*this);

	pTimer = new Timer;
	pTimer->Construct(*this);

	pShadow = new Canvas;
	pShadow->Construct(Rectangle(0, 0, screen_width, screen_height));

	pLabel = static_cast<Label *>(GetControl(L"IDC_LABEL1"));

	style = GetFormStyle();
	ShowSoftkey(SOFTKEY_0, false);
	SetOrientation(ORIENTATION_AUTOMATIC);

	scrollPt = Point(0,0);
	dragStart = Point(-1, -1);
	viewPort = pScroll->GetClientAreaBounds();

	do_defaults("lite", &cfg);
	memset(&ucsdata, 0, sizeof(ucsdata));
	init_ucs(&cfg, &ucsdata);
	cfgtopalette();

	init_fonts(0, cfg.font.height);

	init_palette();
	term = term_init(&cfg, &ucsdata, NULL);

	term_size(term, screen_height/font_height, screen_width/font_width, cfg.savelines);

	skynet = new badaNet;
	skynet->ConstructConnection(false);
	ShowSoftkey(SOFTKEY_0, true);
	return E_SUCCESS;
}

result Form1::OnTerminating(void) {
	result r = E_SUCCESS;

	delete pTimer;
	delete pShadow;
	delete skynet;
	delete pOptions;

	return r;
}

void Form1::OnActionPerformed(const Osp::Ui::Control& source, int actionId) {
	switch (actionId) {
	case ID_BUTTON_EDITFIELD_DONE: {
		if (!term || !ldisc) break;
		char buf[2] = { 0x09, 0x00 };
		term_nopaste(term);
		term_seen_key_event(term);
		ldisc_send(ldisc, buf, 1, 1);
		break;
	}
	case ID_BUTTON_EDITFIELD_CANCEL: {
		typing = false;
		pScroll->CloseOverlayWindow();
		break;
	}
	case ID_SOFTKEY0: {
		ShowSoftkey(SOFTKEY_0, false);
		if (!skynet->IsActive()) {
			int res;
			MessageBox *mb = new MessageBox;
			mb->Construct(L"Connection request", L"Engage Wi-Fi module?", MSGBOX_STYLE_OKCANCEL);
			mb->ShowAndWait(res);
			delete mb;
			if (res == MSGBOX_RESULT_OK) {
				result r = skynet->ConstructConnection(true);
				if (E_SUCCESS == r) {
					const char *msg = "Wi-Fi module engaged\n";
					term_data_untrusted(term, msg, strlen(msg));
				}
			} else {
				free_backend();
				Osp::App::Application::GetInstance()->Terminate();
				break;
			}
		}
		if (back) free_backend();
		for (int i = 0; backends[i].backend != NULL; i++) {
			if (backends[i].protocol == be_default_protocol) {
				back = backends[i].backend;
				break;
			}
		}
		back->init(NULL, &backhandle, &cfg, cfg.host);
		term_provide_resize_fn(term, back->size, backhandle);
		ldisc = ldisc_create(&cfg, term, back, backhandle, 0);
		break;
	}
	case ID_SOFTKEY1:
	break;
	case ID_OPTIONKEY: {
		pOptions->SetShowState(true);
		pOptions->Show();
		break;
	}
	case ID_OPTIONMENU_OPTIONS: {
		optionsForm->LoadConfig(&cfg);
		mainFrame->SetCurrentForm(*optionsForm);
		optionsForm->Draw();
		optionsForm->Show();
		break;
	}
	case ID_OPTIONMENU_DISCONNECT: {
		free_backend();
		break;
	}
	default:
	break;
}
}

void Form1::OnTouchDoublePressed(const Osp::Ui::Control &source, const Osp::Graphics::Point &currentPosition,
		const Osp::Ui::TouchEventInfo &touchInfo) {
	scrollPt = Point(0, 0);
	RequestRedraw(true);
}

void Form1::OnTouchLongPressed(const Osp::Ui::Control &source, const Osp::Graphics::Point &currentPosition,
		const Osp::Ui::TouchEventInfo &touchInfo) {
}

void Form1::OnTouchMoved(const Osp::Ui::Control &source, const Osp::Graphics::Point &currentPosition,
		const Osp::Ui::TouchEventInfo &touchInfo) {
	scrollPt.y = dragStart.y + touchPt.y - currentPosition.y;
	if (scrollPt.y < 0) scrollPt.y = 0;
	if (scrollPt.y > screen_height - viewPort.height) scrollPt.y = screen_height - viewPort.height;
	RequestRedraw(true);
}

void Form1::OnTouchPressed(const Osp::Ui::Control &source, const Osp::Graphics::Point &currentPosition,
		const Osp::Ui::TouchEventInfo &touchInfo) {
	touchPt = currentPosition;
	dragStart = scrollPt;
}

void Form1::OnTouchReleased(const Osp::Ui::Control &source, const Osp::Graphics::Point &currentPosition,
		const Osp::Ui::TouchEventInfo &touchInfo) {
	dragStart = Point(-1, -1);
	Point delta = currentPosition - touchPt;
	if (delta.x * delta.x + delta.y * delta.y < 400 && back && back->sendok(backhandle)) {
		typing = true;
		pEdit->ShowKeypad();
	}
}

void Form1::OnTextValueChanged(const Osp::Ui::Control& source) {
	if (!term || !ldisc) {
		pEdit->SetText("+");
		return;
	}

	wchar_t w[2] = { 0 };
	char buf[2] = { 0 };
	String txt = pEdit->GetText();
	txt.GetCharAt(1, w[0]);
	int lines = pEdit->GetTextLineCount();
	int chars = pEdit->GetTextLength();
	pEdit->SetText("+");

	if (lines > 1 && chars > 1) {
		buf[0] = 0x0D;
		buf[1] = 0;
	} else if (chars > 1) {
		noise_ultralight(w[0]);
		term_nopaste(term);
		term_seen_key_event(term);
		luni_send(ldisc, w, 1, 1);
		return;
	} else if (chars == 1) {
		return;
	} else {
		buf[0] = 0x08;
		buf[1] = 0;
	}

	term_nopaste(term);
	term_seen_key_event(term);
	ldisc_send(ldisc, buf, 1, 1);
}

void Form1::OnTextValueChangeCanceled(const Osp::Ui::Control& source) {

}

result Form1::OnDraw() {
	pEdit->SetShowState(typing);

	if (!pShadow) return E_FAILURE;
	Canvas *pCanvas = GetCanvasN();

	pCanvas->Copy(Point(0, 0), *pShadow, Rectangle(scrollPt.x, scrollPt.y, viewPort.width, viewPort.height));

	delete pCanvas;
	return E_SUCCESS;
}

void Form1::OnOverlayControlClosed(const Osp::Ui::Control &source) {
	scrollPt = Point(0, 0);
	viewPort = pScroll->GetClientAreaBounds();
}

void Form1::ScrollCursor() {
	int cursorY = (term->curs.y + 1) * font_height;
	if (cursorY > viewPort.height) {
		scrollPt.y = dragStart.y = cursorY - viewPort.height;
		RequestRedraw(true);
	}
}

void Form1::OnOverlayControlOpened(const Osp::Ui::Control &source) {
	viewPort = pScroll->GetClientAreaBounds();
	typing = true;
	pEdit->SetText("+");
	ScrollCursor();
}

void Form1::OnOrientationChanged(const Osp::Ui::Control &source, Osp::Ui::OrientationStatus orientationStatus) {
	switch (orientationStatus) {
	case ORIENTATION_STATUS_PORTRAIT:
		screen_width = 480;
		screen_height = 800;
		break;
	case ORIENTATION_STATUS_LANDSCAPE:
		screen_width = 800;
		screen_height = 480;
		break;
	case ORIENTATION_STATUS_LANDSCAPE_REVERSE:
		screen_width = 800;
		screen_height = 480;
		break;
	default:
		break;
	}
	if (pShadow) {
		Canvas *pOld = pShadow;
		pShadow = new Canvas;
		pShadow->Construct(Rectangle(0, 0, screen_width, screen_height));
		pShadow->Copy(Point(0, 0), *pOld, Rectangle(0, 0, 800, 800));
		delete pOld;
	}
	viewPort = pScroll->GetClientAreaBounds();
	pEdit->SetPosition(viewPort.width, -120);
	term_size(term, screen_height / font_height, screen_width / font_width, cfg.savelines);
}

void Form1::OnTimerExpired(Timer &timer) {
	long next;
	started = false;
	if (run_timers(timing_next_time, &next)) timer_change_notify(next);
}

void Form1::SetTimer(long ticks) {
	if (!pTimer) return;
	if (started && E_SUCCESS == pTimer->Cancel()) started = false;
	if (E_SUCCESS == pTimer->Start(ticks)) started = true;
}

void Form1::Redraw() {
	update = true;
	do_paint(term, pShadow, TRUE);
	this->RequestRedraw(true);
}

void Form1::SetStatus(const char *text, Color *color) {
	if (color) pLabel->SetTextColor(*color);
	pLabel->SetText(text);
	pLabel->RequestRedraw(true);
}

void Form1::ShowSoftkey(Osp::Ui::Controls::Softkey key, bool show) {
	FormStyle delta;
	if (key == SOFTKEY_0) delta = FORM_STYLE_SOFTKEY_0;
	else if (key == SOFTKEY_1) delta = FORM_STYLE_SOFTKEY_1;
	else
		return;
	if (show) style |= delta;
	else
		style &= ~delta;
	SetFormStyle(style);
	RequestRedraw(true);
}

void Form1::SocketConnected() {
/*	SetOrientation(ORIENTATION_AUTOMATIC);
	OnOrientationChanged(*this, ORIENTATION_STATUS_PORTRAIT);*/
}

void Form1::ConnectionActive() {
	ShowSoftkey(SOFTKEY_0, true);
}

void Form1::ConnectionStopped() {
/*	SetOrientation(ORIENTATION_PORTRAIT);
	OnOrientationChanged(*this, ORIENTATION_STATUS_PORTRAIT);*/
	ShowSoftkey(SOFTKEY_0, true);
	free_backend();
	SetStatus("Disconnected");
}

void Form1::RemoteExit() {
/*	SetOrientation(ORIENTATION_PORTRAIT);
	OnOrientationChanged(*this, ORIENTATION_STATUS_PORTRAIT);*/
	ShowSoftkey(SOFTKEY_0, true);
	logerror("Host disconnected\n");
	SetStatus("Disconnected");
}

extern "C" void notify_remote_exit(void *frontend) {
	mainForm->RemoteExit();
}

extern "C" void timer_change_notify(long next) {
	long ticks = next - GETTICKCOUNT();
	if (ticks <= 0) ticks = 1;
	mainForm->SetTimer(ticks);
	timing_next_time = next;
}

extern "C" void term_update(Terminal *term) {
	if (!term) return;
	term->window_update_pending = FALSE;
	mainForm->Redraw();
}

extern "C" void logerror(const char *format, ...) {
	if (!term) return;
	va_list list;
	va_start(list, format);
	char buf[2048];
	int len = vsnprintf(buf, 2048, format, list);
	va_end(list);
	term_data_untrusted(term, buf, len);
}

void free_backend() {
	if (ldisc) {
		ldisc_free(ldisc);
		ldisc = NULL;
	}
	if (back) {
		back->free(backhandle);
		backhandle = NULL;
		back = NULL;
		term_provide_resize_fn(term, NULL, NULL);
		update_specials_menu( NULL);
	}
}
