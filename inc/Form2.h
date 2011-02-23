#ifndef _FORM2_H_
#define _FORM2_H_

#include <FBase.h>
#include <FUi.h>

class Form2 :
	public Osp::Ui::Controls::Form,
	public Osp::Ui::IActionEventListener,
 	public Osp::Ui::IKeyEventListener,
 	public Osp::Ui::IAdjustmentEventListener
{
	Osp::Ui::Controls::EditField *pHostname, *pPortnumber, *pUsername, *pPublickey;
	Osp::Ui::Controls::Slider *pFontsize;
	Osp::Ui::Controls::ScrollPanel *pScroll;
	void *pCfg;
	int font;
// Construction
public:
	Form2(void);
	virtual ~Form2(void);
	bool Initialize();
	result OnInitializing(void);
	result OnTerminating(void);
	virtual void OnActionPerformed(const Osp::Ui::Control& source, int actionId);
	virtual result OnDraw();

// Implementation
protected:
    static const int ID_SOFTKEY0 = 10;
    static const int ID_SOFTKEY1 = 11;
	static const int ID_BUTTON_EDITFIELD_DONE = 501;
	static const int ID_BUTTON_EDITFIELD_CANCEL = 502;
// Generated call-back functions
public:
	void LoadConfig(void *cfg);
	void SaveConfig();
	virtual void OnKeyLongPressed(const Osp::Ui::Control &source, Osp::Ui::KeyCode keyCode);
	virtual void OnKeyPressed(const Osp::Ui::Control &source, Osp::Ui::KeyCode keyCode);
	virtual void OnKeyReleased(const Osp::Ui::Control &source, Osp::Ui::KeyCode keyCode);
	virtual void OnAdjustmentValueChanged(const Osp::Ui::Control &source, int adjustment);
};

#endif
