#ifndef _FORM1_H_
#define _FORM1_H_

#include <FBase.h>
#include <FApp.h>
#include <FUi.h>
#include <FIo.h>

class Form1:
	public Osp::Ui::Controls::Form,
	public Osp::Ui::IActionEventListener,
	public Osp::Ui::ITouchEventListener,
	public Osp::Ui::IKeyEventListener,
	public Osp::Ui::ITextEventListener,
	public Osp::Ui::IScrollPanelEventListener,
	public Osp::Base::Runtime::ITimerEventListener,
	public Osp::Ui::IOrientationEventListener {
public:
	Form1(void);
	virtual ~Form1(void);
	bool Initialize(void);
protected:
	Osp::Ui::Controls::ScrollPanel *pScroll;
	Osp::Ui::Controls::EditArea *pEdit;
	Osp::Ui::Controls::Label *pLabel;
	Osp::Ui::Controls::OptionMenu *pOptions;
	Osp::Graphics::Canvas *pShadow;
	Osp::Base::Runtime::Timer *pTimer;
	Osp::Graphics::Point touchPt, scrollPt, dragStart;
	Osp::Graphics::Rectangle viewPort;
	bool update, started, typing;
	int style;
	static const int ID_BUTTON_EDITFIELD_DONE = 501;
	static const int ID_BUTTON_EDITFIELD_CANCEL = 502;
	static const int ID_SOFTKEY0 = 10;
	static const int ID_SOFTKEY1 = 11;
	static const int ID_OPTIONKEY = 100;
	static const int ID_OPTIONMENU_OPTIONS = 101;
	static const int ID_OPTIONMENU_DISCONNECT = 102;
public:
	void SetTimer(long ticks);
	void Redraw();
	void SetStatus(const char*, Osp::Graphics::Color *color = 0);
	void ShowSoftkey(Osp::Ui::Controls::Softkey key, bool show);
	void SocketConnected();
	void ConnectionStopped();
	void ConnectionActive();
	void RemoteExit();
	void ScrollCursor();
	virtual result OnDraw();
	virtual result OnInitializing(void);
	virtual result OnTerminating(void);
	virtual void OnActionPerformed(const Osp::Ui::Control & source, int actionId);
	virtual void OnTouchDoublePressed(const Osp::Ui::Control & source, const Osp::Graphics::Point & currentPosition,
			const Osp::Ui::TouchEventInfo & touchInfo);
	virtual void OnTouchFocusIn(const Osp::Ui::Control & source, const Osp::Graphics::Point & currentPosition,
			const Osp::Ui::TouchEventInfo & touchInfo) {
	}
	virtual void OnTouchFocusOut(const Osp::Ui::Control & source, const Osp::Graphics::Point & currentPosition,
			const Osp::Ui::TouchEventInfo & touchInfo) {
	}

	virtual void OnTouchLongPressed(const Osp::Ui::Control & source, const Osp::Graphics::Point & currentPosition,
			const Osp::Ui::TouchEventInfo & touchInfo);
	virtual void OnTouchMoved(const Osp::Ui::Control & source, const Osp::Graphics::Point & currentPosition,
			const Osp::Ui::TouchEventInfo & touchInfo);
	virtual void OnTouchPressed(const Osp::Ui::Control & source, const Osp::Graphics::Point & currentPosition,
			const Osp::Ui::TouchEventInfo & touchInfo);
	virtual void OnTouchReleased(const Osp::Ui::Control & source, const Osp::Graphics::Point & currentPosition,
			const Osp::Ui::TouchEventInfo & touchInfo);
	virtual void OnKeyLongPressed(const Osp::Ui::Control & source, Osp::Ui::KeyCode keyCode) {
	}
	virtual void OnKeyPressed(const Osp::Ui::Control & source, Osp::Ui::KeyCode keyCode) {
	}
	virtual void OnKeyReleased(const Osp::Ui::Control & source, Osp::Ui::KeyCode keyCode) {
	}
	virtual void OnTextValueChanged(const Osp::Ui::Control & source);
	virtual void OnTextValueChangeCanceled(const Osp::Ui::Control & source);
	virtual void OnTimerExpired(Osp::Base::Runtime::Timer & timer);
	virtual void OnOtherControlSelected(const Osp::Ui::Control & source) {
	}
	virtual void OnOverlayControlClosed(const Osp::Ui::Control & source);
	virtual void OnOverlayControlCreated(const Osp::Ui::Control & source) {
	}
	virtual void OnOverlayControlOpened(const Osp::Ui::Control & source);
	virtual void OnOrientationChanged(const Osp::Ui::Control & source, Osp::Ui::OrientationStatus orientationStatus);
};

#endif	//_FORM1_H_
