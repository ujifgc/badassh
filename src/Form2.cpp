extern "C" {
#include "putty.h"
#include "terminal.h"
}

#include <stdlib.h>
#include "Form1.h"
#include "Form2.h"

using namespace Osp::Base;
using namespace Osp::Ui;
using namespace Osp::Ui::Controls;
using namespace Osp::Graphics;

extern Form1 *mainForm;
extern Form2 *optionsForm;
extern Frame *mainFrame;
extern int font_width, font_height;
extern int screen_width, screen_height;
extern Terminal *term;

void init_fonts(int, int);

Form2::Form2(void) {
}

Form2::~Form2(void) {
}

bool Form2::Initialize() {
	Form::Construct(L"IDF_FORM2");

	return true;
}

result Form2::OnInitializing(void) {
	result r = E_SUCCESS;

	AddKeyEventListener(*this);

	SetSoftkeyActionId(SOFTKEY_0, ID_SOFTKEY0);
	SetSoftkeyActionId(SOFTKEY_1, ID_SOFTKEY1);
	AddSoftkeyActionListener(SOFTKEY_0, *this);
	AddSoftkeyActionListener(SOFTKEY_1, *this);

	pScroll = static_cast<ScrollPanel *> (GetControl(L"IDC_SCROLLPANEL1"));

	pHostname = static_cast<EditField *>(GetControl(L"IDPC_EDITFIELD1", true));
	pPortnumber = static_cast<EditField *>(GetControl(L"IDPC_EDITFIELD3", true));
	pUsername = static_cast<EditField *>(GetControl(L"IDPC_EDITFIELD2", true));
	pPublickey = static_cast<EditField *>(GetControl(L"IDPC_EDITFIELD4", true));
	pFontsize = static_cast<Slider *> (GetControl(L"IDPC_SLIDER1", true));
	pHostname->AddActionEventListener(*this);
	pPortnumber->AddActionEventListener(*this);
	pUsername->AddActionEventListener(*this);
	pPublickey->AddActionEventListener(*this);
	pFontsize->AddAdjustmentEventListener(*this);

	pHostname->SetLowerCaseModeEnabled(true);
	pUsername->SetLowerCaseModeEnabled(true);
	pPortnumber->SetCurrentInputModeCategory(EDIT_INPUTMODE_NUMERIC);
	pPortnumber->SetInputModeCategory(EDIT_INPUTMODE_ALPHA|EDIT_INPUTMODE_PREDICTIVE|EDIT_INPUTMODE_SYMBOL, false);

	pHostname->SetOverlayKeypadCommandButton(COMMAND_BUTTON_POSITION_LEFT, L"Done", ID_BUTTON_EDITFIELD_DONE);
	pHostname->SetOverlayKeypadCommandButton(COMMAND_BUTTON_POSITION_RIGHT, L"Cancel", ID_BUTTON_EDITFIELD_CANCEL);
	pPortnumber->SetOverlayKeypadCommandButton(COMMAND_BUTTON_POSITION_LEFT, L"Done", ID_BUTTON_EDITFIELD_DONE);
	pPortnumber->SetOverlayKeypadCommandButton(COMMAND_BUTTON_POSITION_RIGHT, L"Cancel", ID_BUTTON_EDITFIELD_CANCEL);
	pUsername->SetOverlayKeypadCommandButton(COMMAND_BUTTON_POSITION_LEFT, L"Done", ID_BUTTON_EDITFIELD_DONE);
	pUsername->SetOverlayKeypadCommandButton(COMMAND_BUTTON_POSITION_RIGHT, L"Cancel", ID_BUTTON_EDITFIELD_CANCEL);
	pPublickey->SetOverlayKeypadCommandButton(COMMAND_BUTTON_POSITION_LEFT, L"Done", ID_BUTTON_EDITFIELD_DONE);
	pPublickey->SetOverlayKeypadCommandButton(COMMAND_BUTTON_POSITION_RIGHT, L"Cancel", ID_BUTTON_EDITFIELD_CANCEL);

	return r;
}

result Form2::OnTerminating(void) {
	result r = E_SUCCESS;

	// TODO: Add your termination code here

	return r;
}

void Form2::OnActionPerformed(const Osp::Ui::Control& source, int actionId) {
	switch (actionId) {
	case ID_SOFTKEY0:
		SaveConfig();
		mainFrame->SetCurrentForm(*mainForm);
		mainForm->Redraw();
		mainForm->Draw();
		mainForm->Show();
		break;
	case ID_SOFTKEY1:
		mainFrame->SetCurrentForm(*mainForm);
		mainForm->Redraw();
		mainForm->Draw();
		mainForm->Show();
		break;
	case ID_BUTTON_EDITFIELD_DONE: {
		pScroll->CloseOverlayWindow();
		break;
	}
	case ID_BUTTON_EDITFIELD_CANCEL:
		pScroll->CloseOverlayWindow();
		break;
	}
}

void Form2::OnKeyLongPressed(const Osp::Ui::Control &source, Osp::Ui::KeyCode keyCode) {
	// TODO: Add your implementation codes here

}

void Form2::OnKeyPressed(const Osp::Ui::Control &source, Osp::Ui::KeyCode keyCode) {
	// TODO: Add your implementation codes here

}

void Form2::OnKeyReleased(const Osp::Ui::Control &source, Osp::Ui::KeyCode keyCode) {
	// TODO: Add your implementation codes here

}

void Form2::LoadConfig(void *_cfg) {
	pCfg = _cfg;
	Config *cfg = (Config *) pCfg;

	pHostname->SetText(cfg->host);
	pPortnumber->SetText(Integer::ToString(cfg->port));
	pUsername->SetText(cfg->username);
	pPublickey->SetText(cfg->keyfile.path);
	pFontsize->SetValue(cfg->font.height);
	font = pFontsize->GetValue();
}

void Form2::SaveConfig() {
	Config *cfg = (Config *) pCfg;
	String s;

	s = pHostname->GetText();
	wcstombs(cfg->host, s.GetPointer(), sizeof(cfg->host));

	s = pPortnumber->GetText();
	Integer::Parse(s, cfg->port);
	if (cfg->port <= 0 || cfg->port > 65535)
		cfg->port = 22;

	s = pUsername->GetText();
	wcstombs(cfg->username, s.GetPointer(), sizeof(cfg->username));

	s = pPublickey->GetText();
	wcstombs(cfg->keyfile.path, s.GetPointer(), sizeof(cfg->keyfile.path));

	cfg->font.height = pFontsize->GetValue();

	save_settings("lite", cfg);

	init_fonts(0, cfg->font.height);
	term_size(term, screen_height/font_height, screen_width/font_width, cfg->savelines);
}

static const char* legal[] = {
		"SSH client app for bada platform",
		"Based on PuTTY SSH client",
		"This app uses Liberation Mono font",
};

void Form2::OnAdjustmentValueChanged(const Osp::Ui::Control &source, int adjustment) {
	font = pFontsize->GetValue();
	RequestRedraw(true);
}

result Form2::OnDraw() {
	Canvas *pCanvas = GetCanvasN();
	Font *pFont = new Font;
	result r = pFont->Construct("/Res/LiberationMono-Regular.ttf", FONT_STYLE_PLAIN, font);
	pCanvas->SetFont(*pFont);
	for (int i = 0; i < 3; i++)
		pCanvas->DrawText(Point(10,600+i*font),legal[i]);
	delete pCanvas;
	delete pFont;
	r = E_SUCCESS;
	return r;
}
