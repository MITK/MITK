#ifndef EVENTMAPPER_H_HEADER_INCLUDED_C187864A
#define EVENTMAPPER_H_HEADER_INCLUDED_C187864A

#include "mitkCommon.h"
#include "Event.h"
#include "EventDescription.h"
#include "StateEvent.h"
#include "StateMachine.h"
#include <qxml.h>
#include <vector>
#include <string>

//##ModelId=3E788FC0001A
typedef std::vector<mitk::EventDescription> EventDescriptionVec;

//##ModelId=3EAEC3AE01DF
typedef std::vector<mitk::EventDescription>::iterator EventDescriptionVecIter;

//##ModelId=3ECA2D3D0035
struct StringIntArrayStruct
{//for mapping string from xml-file to int
    //##ModelId=3ECA2D3D01F9
	const char * text;
    //##ModelId=3ECA2D3D0202
	int id;
};

namespace mitk {

//##ModelId=3E5A390401F2
class EventMapper : public QXmlDefaultHandler
{
  public:
    //##ModelId=3E5B349600CB
    //##Documentation
	//## set the global StateMachine. If temporaryly changed,
	//## then copy the old statemachine with GetStateMachine()
    static void SetGlobalStateMachine(StateMachine* stateMachine);

    //##ModelId=3EDAFB81012E
	static StateMachine* GetGlobalStateMachine();

    //##ModelId=3E5B34CF0041
	//##Documentation
	//## searches the Event in m_EventDescription
	//## and if included
    static bool MapEvent(Event* event);

    //##ModelId=3E5B35140072
	//##Documentation
	//## loads an XML-File containing Events into m_EventDescriptions
	//## also involved: EventMapper::startEvent(...)
	static bool LoadBehavior(std::string fileName);

    //##ModelId=3E788FC00308
	//##Documentation
	//## reads a Tag from an XML-file
	//## adds Events to m_EventDescription
	bool startElement( const QString&, const QString&, const QString& qName, const QXmlAttributes& atts );

	//##ModelId=3E7B20EE01F5
    std::string GetStyleName();


  private:
    //##ModelId=3E5B33F303CA
	static EventDescriptionVec m_EventDescriptions;

    //##ModelId=3E5B343701F1
    static StateMachine *m_GlobalStateMachine;

	//##ModelId=3E956E3A036B
    static StateEvent m_StateEvent;

	//##ModelId=3E7B1EB800CC
    static std::string m_StyleName;

    //##ModelId=3E788FC000E5
	static const std::string STYLE;
    //##ModelId=3E788FC0025C
	static const std::string NAME;
    //##ModelId=3E788FC002AA
	static const std::string ID;
    //##ModelId=3E785B1B00FD
	static const std::string TYPE;
    //##ModelId=3E785B1B015B
	static const std::string BUTTON;
    //##ModelId=3E8B08FA01AA
    static const std::string BUTTONSTATE;
    //##ModelId=3E785B1B01A9
	static const std::string KEY;
};
} // namespace mitk

#endif /* EVENTMAPPER_H_HEADER_INCLUDED_C187864A */


//--------------------------------------
//outquoted because of the use of the Visual Compiler 6.0, which doesn't support
//switch-case with constant... is to be changed!
/*
public:
	//##Constants for EventMapping... In StateMachines do: case EventMapper::KeyPress
	//##SideEffectId

	static const int mitk::EventMapper::DONOTHING = 0;
	static const int mitk::EventMapper::INITNEWOBJECT = 5;
	static const int mitk::EventMapper::INITEDITOBJECT = 6;
	static const int mitk::EventMapper::INITEDITGROUP = 7;
	static const int mitk::EventMapper::ADDPOINT = 10;
	static const int mitk::EventMapper::SNAPPING_ADDING = 20;
	static const int mitk::EventMapper::CHECKPOINT = 30;
	static const int mitk::EventMapper::CHECKIFLAST = 31;
	static const int mitk::EventMapper::FINISHOBJECT = 40;
	static const int mitk::EventMapper::FINISHGROUP = 41;
	static const int mitk::EventMapper::SEARCHOBJECT = 50;
	static const int mitk::EventMapper::SEARCHGROUP = 51;
	static const int mitk::EventMapper::SEARCHANOTHEROBJECT = 52; //one object is selected and another object is to be added to selection
	static const int mitk::EventMapper::SELECTPICKEDOBJECT = 60; // select the picked object and deselect others
	static const int mitk::EventMapper::SELECTANOTHEROBJECT = 61; //adds the picked object to the selection
	static const int mitk::EventMapper::SELECTGROUP = 62;
    static const int mitk::EventMapper::SELECTALL = 63;
	static const int mitk::EventMapper::ADDSELECTEDTOGROUP = 63;
	static const int mitk::EventMapper::DESELECTOBJECT = 70; //deselect picked from group
	static const int mitk::EventMapper::DESELECTALL = 72;

	//Type
	static const int mitk::EventMapper::None = 0;				// invalid event
	static const int mitk::EventMapper::Timer = 1;				// timer event
	static const int mitk::EventMapper::MouseButtonPress = 2;			// mouse button pressed
	static const int mitk::EventMapper::MouseButtonRelease = 3;			// mouse button released
	static const int mitk::EventMapper::MouseButtonDblClick = 4;		// mouse button double click
	static const int mitk::EventMapper::MouseMove = 5;				// mouse move
	static const int mitk::EventMapper::KeyPress = 6;				// key pressed
	static const int mitk::EventMapper::KeyRelease = 7;				// key released
	static const int mitk::EventMapper::FocusIn = 8;				// keyboard focus received
	static const int mitk::EventMapper::FocusOut = 9;				// keyboard focus lost
	static const int mitk::EventMapper::Enter = 10;				// mouse enters widget
	static const int mitk::EventMapper::Leave = 11;				// mouse leaves widget
	static const int mitk::EventMapper::Paint = 12;				// paint widget
	static const int mitk::EventMapper::Move = 13;				// move widget
	static const int mitk::EventMapper::Resize = 14;				// resize widget
	static const int mitk::EventMapper::Create = 15;				// after object creation
	static const int mitk::EventMapper::Destroy = 16;				// during object destruction
	static const int mitk::EventMapper::Show = 17;				// widget is shown
	static const int mitk::EventMapper::Hide = 18;				// widget is hidden
	static const int mitk::EventMapper::Close = 19;				// request to close widget
	static const int mitk::EventMapper::Quit = 20;				// request to quit application
	static const int mitk::EventMapper::Reparent = 21;				// widget has been reparented
	static const int mitk::EventMapper::ShowMinimized = 22;		       	// widget is shown minimized
	static const int mitk::EventMapper::ShowNormal = 23;	       		// widget is shown normal
	static const int mitk::EventMapper::WindowActivate = 24;	       		// window was activated
	static const int mitk::EventMapper::WindowDeactivate = 25;	       		// window was deactivated
	static const int mitk::EventMapper::ShowToParent = 26;	       		// widget is shown to parent
	static const int mitk::EventMapper::HideToParent = 27;	       		// widget is hidden to parent
	static const int mitk::EventMapper::ShowMaximized = 28;		       	// widget is shown maximized
	static const int mitk::EventMapper::ShowFullScreen = 29;			// widget is shown full-screen
	static const int mitk::EventMapper::Accel = 30;				// accelerator event
	static const int mitk::EventMapper::Wheel = 31;				// wheel event
	static const int mitk::EventMapper::AccelAvailable = 32;			// accelerator available event
	static const int mitk::EventMapper::CaptionChange = 33;			// caption changed
	static const int mitk::EventMapper::IconChange = 34;			// icon changed
	static const int mitk::EventMapper::ParentFontChange = 35;			// parent font changed
	static const int mitk::EventMapper::ApplicationFontChange = 36;		// application font changed
	static const int mitk::EventMapper::ParentPaletteChange = 37;		// parent palette changed
	static const int mitk::EventMapper::ApplicationPaletteChange = 38;		// application palette changed
	static const int mitk::EventMapper::PaletteChange = 39;			// widget palette changed
	static const int mitk::EventMapper::Clipboard = 40;				// internal clipboard event
	static const int mitk::EventMapper::Speech = 42;				// reserved for speech input
	static const int mitk::EventMapper::SockAct = 50;				// socket activation
	static const int mitk::EventMapper::AccelOverride = 51;			// accelerator override event
	static const int mitk::EventMapper::DeferredDelete = 52;			// deferred delete event
	static const int mitk::EventMapper::DragEnter = 60;				// drag moves into widget
	static const int mitk::EventMapper::DragMove = 61;				// drag moves in widget
	static const int mitk::EventMapper::DragLeave = 62;				// drag leaves or is cancelled
	static const int mitk::EventMapper::Drop = 63;				// actual drop
	static const int mitk::EventMapper::DragResponse = 64;			// drag accepted/rejected
	static const int mitk::EventMapper::ChildInserted = 70;			// new child widget
	static const int mitk::EventMapper::ChildRemoved = 71;			// deleted child widget
	static const int mitk::EventMapper::LayoutHint = 72;			// child min/max size changed
	static const int mitk::EventMapper::ShowWindowRequest = 73;			// widget's window should be mapped
	static const int mitk::EventMapper::ActivateControl = 80;			// ActiveX activation
	static const int mitk::EventMapper::DeactivateControl = 81;			// ActiveX deactivation
	static const int mitk::EventMapper::ContextMenu = 82;			// context popup menu
	static const int mitk::EventMapper::IMStart = 83;				// input method composition start
	static const int mitk::EventMapper::IMCompose = 84;				// input method composition
	static const int mitk::EventMapper::IMEnd = 85;				// input method composition end
	static const int mitk::EventMapper::Accessibility = 86;			// accessibility information is requested
	static const int mitk::EventMapper::TabletMove = 87;			// Wacom tablet event
	static const int mitk::EventMapper::LocaleChange = 88;			// the system locale changed
	static const int mitk::EventMapper::LanguageChange = 89;			// the application language changed
	static const int mitk::EventMapper::LayoutDirectionChange = 90;		// the layout direction changed
	static const int mitk::EventMapper::Style = 91;				// internal style event
	static const int mitk::EventMapper::TabletPress = 92;			// tablet press
	static const int mitk::EventMapper::TabletRelease = 93;			// tablet release
	static const int mitk::EventMapper::User = 1000;				// first user event id
	static const int mitk::EventMapper::MaxUser = 65535;

	//##ButtonState
	// mouse/keyboard state values
    static const int mitk::EventMapper::NoButton        = 0x0000;
    static const int mitk::EventMapper::LeftButton      = 0x0001;
    static const int mitk::EventMapper::RightButton     = 0x0002;
    static const int mitk::EventMapper::MidButton       = 0x0004;
    static const int mitk::EventMapper::MouseButtonMask = 0x0007;
    static const int mitk::EventMapper::ShiftButton     = 0x0008;
    static const int mitk::EventMapper::ControlButton   = 0x0010;
    static const int mitk::EventMapper::AltButton       = 0x0020;
    static const int mitk::EventMapper::KeyButtonMask   = 0x0038;
    static const int mitk::EventMapper::Keypad          = 0x4000;

	//##Key
    static const int mitk::EventMapper::Key_Escape = 0x1000;            // misc keys
    static const int mitk::EventMapper::Key_Tab = 0x1001;
    static const int mitk::EventMapper::Key_Backtab = 0x1002;
	static const int mitk::EventMapper::Key_BackTab = Key_Backtab;
    static const int mitk::EventMapper::Key_Backspace = 0x1003;
	static const int mitk::EventMapper::Key_BackSpace = Key_Backspace;
    static const int mitk::EventMapper::Key_Return = 0x1004;
    static const int mitk::EventMapper::Key_Enter = 0x1005;
    static const int mitk::EventMapper::Key_Insert = 0x1006;
    static const int mitk::EventMapper::Key_Delete = 0x1007;
    static const int mitk::EventMapper::Key_Pause = 0x1008;
    static const int mitk::EventMapper::Key_Print = 0x1009;
    static const int mitk::EventMapper::Key_SysReq = 0x100a;
    static const int mitk::EventMapper::Key_Home = 0x1010;              // cursor movement
    static const int mitk::EventMapper::Key_End = 0x1011;
    static const int mitk::EventMapper::Key_Left = 0x1012;
    static const int mitk::EventMapper::Key_Up = 0x1013;
    static const int mitk::EventMapper::Key_Right = 0x1014;
    static const int mitk::EventMapper::Key_Down = 0x1015;
    static const int mitk::EventMapper::Key_Prior = 0x1016;
	static const int mitk::EventMapper::Key_PageUp = Key_Prior;
    static const int mitk::EventMapper::Key_Next = 0x1017;
	static const int mitk::EventMapper::Key_PageDown = Key_Next;
    static const int mitk::EventMapper::Key_Shift = 0x1020;             // modifiers
    static const int mitk::EventMapper::Key_Control = 0x1021;
    static const int mitk::EventMapper::Key_Meta = 0x1022;
    static const int mitk::EventMapper::Key_Alt = 0x1023;
    static const int mitk::EventMapper::Key_CapsLock = 0x1024;
    static const int mitk::EventMapper::Key_NumLock = 0x1025;
    static const int mitk::EventMapper::Key_ScrollLock = 0x1026;
    static const int mitk::EventMapper::Key_F1 = 0x1030;                // function keys
    static const int mitk::EventMapper::Key_F2 = 0x1031;
    static const int mitk::EventMapper::Key_F3 = 0x1032;
    static const int mitk::EventMapper::Key_F4 = 0x1033;
    static const int mitk::EventMapper::Key_F5 = 0x1034;
    static const int mitk::EventMapper::Key_F6 = 0x1035;
    static const int mitk::EventMapper::Key_F7 = 0x1036;
    static const int mitk::EventMapper::Key_F8 = 0x1037;
    static const int mitk::EventMapper::Key_F9 = 0x1038;
    static const int mitk::EventMapper::Key_F10 = 0x1039;
    static const int mitk::EventMapper::Key_F11 = 0x103a;
    static const int mitk::EventMapper::Key_F12 = 0x103b;
    static const int mitk::EventMapper::Key_F13 = 0x103c;
    static const int mitk::EventMapper::Key_F14 = 0x103d;
    static const int mitk::EventMapper::Key_F15 = 0x103e;
    static const int mitk::EventMapper::Key_F16 = 0x103f;
    static const int mitk::EventMapper::Key_F17 = 0x1040;
    static const int mitk::EventMapper::Key_F18 = 0x1041;
    static const int mitk::EventMapper::Key_F19 = 0x1042;
    static const int mitk::EventMapper::Key_F20 = 0x1043;
    static const int mitk::EventMapper::Key_F21 = 0x1044;
    static const int mitk::EventMapper::Key_F22 = 0x1045;
    static const int mitk::EventMapper::Key_F23 = 0x1046;
    static const int mitk::EventMapper::Key_F24 = 0x1047;
    static const int mitk::EventMapper::Key_F25 = 0x1048;               // F25 .. F35 only on X11
    static const int mitk::EventMapper::Key_F26 = 0x1049;
    static const int mitk::EventMapper::Key_F27 = 0x104a;
    static const int mitk::EventMapper::Key_F28 = 0x104b;
    static const int mitk::EventMapper::Key_F29 = 0x104c;
    static const int mitk::EventMapper::Key_F30 = 0x104d;
    static const int mitk::EventMapper::Key_F31 = 0x104e;
    static const int mitk::EventMapper::Key_F32 = 0x104f;
    static const int mitk::EventMapper::Key_F33 = 0x1050;
    static const int mitk::EventMapper::Key_F34 = 0x1051;
    static const int mitk::EventMapper::Key_F35 = 0x1052;
    static const int mitk::EventMapper::Key_Super_L = 0x1053;           // extra keys
    static const int mitk::EventMapper::Key_Super_R = 0x1054;
    static const int mitk::EventMapper::Key_Menu = 0x1055;
    static const int mitk::EventMapper::Key_Hyper_L = 0x1056;
    static const int mitk::EventMapper::Key_Hyper_R = 0x1057;
    static const int mitk::EventMapper::Key_Help = 0x1058;
        // International input method support (X keycode - 0xEE00)
        // Only interesting if you are writing your own input method
    static const int mitk::EventMapper::Key_Muhenkan =  0x1122;  // Cancel Conversion
    static const int mitk::EventMapper::Key_Henkan =  0x1123;  // Start/Stop Conversion
    static const int mitk::EventMapper::Key_Hiragana_Katakana = 0x1127;  // Hiragana/Katakana toggle
    static const int mitk::EventMapper::Key_Zenkaku_Hankaku = 0x112A;  // Zenkaku/Hankaku toggle
    static const int mitk::EventMapper::Key_Space = 0x20;               // 7 bit printable ASCII
    static const int mitk::EventMapper::Key_Any = Key_Space;
    static const int mitk::EventMapper::Key_Exclam = 0x21;
    static const int mitk::EventMapper::Key_QuoteDbl = 0x22;
    static const int mitk::EventMapper::Key_NumberSign = 0x23;
    static const int mitk::EventMapper::Key_Dollar = 0x24;
    static const int mitk::EventMapper::Key_Percent = 0x25;
    static const int mitk::EventMapper::Key_Ampersand = 0x26;
    static const int mitk::EventMapper::Key_Apostrophe = 0x27;
    static const int mitk::EventMapper::Key_ParenLeft = 0x28;
    static const int mitk::EventMapper::Key_ParenRight = 0x29;
    static const int mitk::EventMapper::Key_Asterisk = 0x2a;
    static const int mitk::EventMapper::Key_Plus = 0x2b;
    static const int mitk::EventMapper::Key_Comma = 0x2c;
    static const int mitk::EventMapper::Key_Minus = 0x2d;
    static const int mitk::EventMapper::Key_Period = 0x2e;
    static const int mitk::EventMapper::Key_Slash = 0x2f;
    static const int mitk::EventMapper::Key_0 = 0x30;
    static const int mitk::EventMapper::Key_1 = 0x31;
    static const int mitk::EventMapper::Key_2 = 0x32;
    static const int mitk::EventMapper::Key_3 = 0x33;
    static const int mitk::EventMapper::Key_4 = 0x34;
    static const int mitk::EventMapper::Key_5 = 0x35;
    static const int mitk::EventMapper::Key_6 = 0x36;
    static const int mitk::EventMapper::Key_7 = 0x37;
    static const int mitk::EventMapper::Key_8 = 0x38;
    static const int mitk::EventMapper::Key_9 = 0x39;
    static const int mitk::EventMapper::Key_Colon = 0x3a;
    static const int mitk::EventMapper::Key_Semicolon = 0x3b;
    static const int mitk::EventMapper::Key_Less = 0x3c;
    static const int mitk::EventMapper::Key_Equal = 0x3d;
    static const int mitk::EventMapper::Key_Greater = 0x3e;
    static const int mitk::EventMapper::Key_Question = 0x3f;
    static const int mitk::EventMapper::Key_At = 0x40;
    static const int mitk::EventMapper::Key_A = 0x41;
    static const int mitk::EventMapper::Key_B = 0x42;
    static const int mitk::EventMapper::Key_C = 0x43;
    static const int mitk::EventMapper::Key_D = 0x44;
    static const int mitk::EventMapper::Key_E = 0x45;
    static const int mitk::EventMapper::Key_F = 0x46;
    static const int mitk::EventMapper::Key_G = 0x47;
    static const int mitk::EventMapper::Key_H = 0x48;
    static const int mitk::EventMapper::Key_I = 0x49;
    static const int mitk::EventMapper::Key_J = 0x4a;
    static const int mitk::EventMapper::Key_K = 0x4b;
    static const int mitk::EventMapper::Key_L = 0x4c;
    static const int mitk::EventMapper::Key_M = 0x4d;
    static const int mitk::EventMapper::Key_N = 0x4e;
    static const int mitk::EventMapper::Key_O = 0x4f;
    static const int mitk::EventMapper::Key_P = 0x50;
    static const int mitk::EventMapper::Key_Q = 0x51;
    static const int mitk::EventMapper::Key_R = 0x52;
    static const int mitk::EventMapper::Key_S = 0x53;
    static const int mitk::EventMapper::Key_T = 0x54;
    static const int mitk::EventMapper::Key_U = 0x55;
    static const int mitk::EventMapper::Key_V = 0x56;
    static const int mitk::EventMapper::Key_W = 0x57;
    static const int mitk::EventMapper::Key_X = 0x58;
    static const int mitk::EventMapper::Key_Y = 0x59;
    static const int mitk::EventMapper::Key_Z = 0x5a;
    static const int mitk::EventMapper::Key_BracketLeft = 0x5b;
    static const int mitk::EventMapper::Key_Backslash = 0x5c;
    static const int mitk::EventMapper::Key_BracketRight = 0x5d;
    static const int mitk::EventMapper::Key_AsciiCircum = 0x5e;
    static const int mitk::EventMapper::Key_Underscore = 0x5f;
    static const int mitk::EventMapper::Key_QuoteLeft = 0x60;
    static const int mitk::EventMapper::Key_BraceLeft = 0x7b;
    static const int mitk::EventMapper::Key_Bar = 0x7c;
    static const int mitk::EventMapper::Key_BraceRight = 0x7d;
    static const int mitk::EventMapper::Key_AsciiTilde = 0x7e;
    static const int mitk::EventMapper::Key_nobreakspace = 0x0a0;
    static const int mitk::EventMapper::Key_exclamdown = 0x0a1;
    static const int mitk::EventMapper::Key_cent = 0x0a2;
    static const int mitk::EventMapper::Key_sterling = 0x0a3;
    static const int mitk::EventMapper::Key_currency = 0x0a4;
    static const int mitk::EventMapper::Key_yen = 0x0a5;
    static const int mitk::EventMapper::Key_brokenbar = 0x0a6;
    static const int mitk::EventMapper::Key_section = 0x0a7;
    static const int mitk::EventMapper::Key_diaeresis = 0x0a8;
    static const int mitk::EventMapper::Key_copyright = 0x0a9;
    static const int mitk::EventMapper::Key_ordfeminine = 0x0aa;
    static const int mitk::EventMapper::Key_guillemotleft = 0x0ab;      // left angle quotation mark
    static const int mitk::EventMapper::Key_notsign = 0x0ac;
    static const int mitk::EventMapper::Key_hyphen = 0x0ad;
    static const int mitk::EventMapper::Key_registered = 0x0ae;
    static const int mitk::EventMapper::Key_macron = 0x0af;
    static const int mitk::EventMapper::Key_degree = 0x0b0;
    static const int mitk::EventMapper::Key_plusminus = 0x0b1;
    static const int mitk::EventMapper::Key_twosuperior = 0x0b2;
    static const int mitk::EventMapper::Key_threesuperior = 0x0b3;
    static const int mitk::EventMapper::Key_acute = 0x0b4;
    static const int mitk::EventMapper::Key_mu = 0x0b5;
    static const int mitk::EventMapper::Key_paragraph = 0x0b6;
    static const int mitk::EventMapper::Key_periodcentered = 0x0b7;
    static const int mitk::EventMapper::Key_cedilla = 0x0b8;
    static const int mitk::EventMapper::Key_onesuperior = 0x0b9;
    static const int mitk::EventMapper::Key_masculine = 0x0ba;
    static const int mitk::EventMapper::Key_guillemotright = 0x0bb;     // right angle quotation mark
    static const int mitk::EventMapper::Key_onequarter = 0x0bc;
    static const int mitk::EventMapper::Key_onehalf = 0x0bd;
    static const int mitk::EventMapper::Key_threequarters = 0x0be;
    static const int mitk::EventMapper::Key_questiondown = 0x0bf;
    static const int mitk::EventMapper::Key_Agrave = 0x0c0;
    static const int mitk::EventMapper::Key_Aacute = 0x0c1;
    static const int mitk::EventMapper::Key_Acircumflex = 0x0c2;
    static const int mitk::EventMapper::Key_Atilde = 0x0c3;
    static const int mitk::EventMapper::Key_Adiaeresis = 0x0c4;
    static const int mitk::EventMapper::Key_Aring = 0x0c5;
    static const int mitk::EventMapper::Key_AE = 0x0c6;
    static const int mitk::EventMapper::Key_Ccedilla = 0x0c7;
    static const int mitk::EventMapper::Key_Egrave = 0x0c8;
    static const int mitk::EventMapper::Key_Eacute = 0x0c9;
    static const int mitk::EventMapper::Key_Ecircumflex = 0x0ca;
    static const int mitk::EventMapper::Key_Ediaeresis = 0x0cb;
    static const int mitk::EventMapper::Key_Igrave = 0x0cc;
    static const int mitk::EventMapper::Key_Iacute = 0x0cd;
    static const int mitk::EventMapper::Key_Icircumflex = 0x0ce;
    static const int mitk::EventMapper::Key_Idiaeresis = 0x0cf;
    static const int mitk::EventMapper::Key_ETH = 0x0d0;
    static const int mitk::EventMapper::Key_Ntilde = 0x0d1;
    static const int mitk::EventMapper::Key_Ograve = 0x0d2;
    static const int mitk::EventMapper::Key_Oacute = 0x0d3;
    static const int mitk::EventMapper::Key_Ocircumflex = 0x0d4;
    static const int mitk::EventMapper::Key_Otilde = 0x0d5;
    static const int mitk::EventMapper::Key_Odiaeresis = 0x0d6;
    static const int mitk::EventMapper::Key_multiply = 0x0d7;
    static const int mitk::EventMapper::Key_Ooblique = 0x0d8;
    static const int mitk::EventMapper::Key_Ugrave = 0x0d9;
    static const int mitk::EventMapper::Key_Uacute = 0x0da;
    static const int mitk::EventMapper::Key_Ucircumflex = 0x0db;
    static const int mitk::EventMapper::Key_Udiaeresis = 0x0dc;
    static const int mitk::EventMapper::Key_Yacute = 0x0dd;
    static const int mitk::EventMapper::Key_THORN = 0x0de;
    static const int mitk::EventMapper::Key_ssharp = 0x0df;
    static const int mitk::EventMapper::Key_agrave = 0x0e0;
    static const int mitk::EventMapper::Key_aacute = 0x0e1;
    static const int mitk::EventMapper::Key_acircumflex = 0x0e2;
    static const int mitk::EventMapper::Key_atilde = 0x0e3;
    static const int mitk::EventMapper::Key_adiaeresis = 0x0e4;
    static const int mitk::EventMapper::Key_aring = 0x0e5;
    static const int mitk::EventMapper::Key_ae = 0x0e6;
    static const int mitk::EventMapper::Key_ccedilla = 0x0e7;
    static const int mitk::EventMapper::Key_egrave = 0x0e8;
    static const int mitk::EventMapper::Key_eacute = 0x0e9;
    static const int mitk::EventMapper::Key_ecircumflex = 0x0ea;
    static const int mitk::EventMapper::Key_ediaeresis = 0x0eb;
    static const int mitk::EventMapper::Key_igrave = 0x0ec;
    static const int mitk::EventMapper::Key_iacute = 0x0ed;
    static const int mitk::EventMapper::Key_icircumflex = 0x0ee;
    static const int mitk::EventMapper::Key_idiaeresis = 0x0ef;
    static const int mitk::EventMapper::Key_eth = 0x0f0;
    static const int mitk::EventMapper::Key_ntilde = 0x0f1;
    static const int mitk::EventMapper::Key_ograve = 0x0f2;
    static const int mitk::EventMapper::Key_oacute = 0x0f3;
    static const int mitk::EventMapper::Key_ocircumflex = 0x0f4;
    static const int mitk::EventMapper::Key_otilde = 0x0f5;
    static const int mitk::EventMapper::Key_odiaeresis = 0x0f6;
    static const int mitk::EventMapper::Key_division = 0x0f7;
    static const int mitk::EventMapper::Key_oslash = 0x0f8;
    static const int mitk::EventMapper::Key_ugrave = 0x0f9;
    static const int mitk::EventMapper::Key_uacute = 0x0fa;
    static const int mitk::EventMapper::Key_ucircumflex = 0x0fb;
    static const int mitk::EventMapper::Key_udiaeresis = 0x0fc;
    static const int mitk::EventMapper::Key_yacute = 0x0fd;
    static const int mitk::EventMapper::Key_thorn = 0x0fe;
    static const int mitk::EventMapper::Key_ydiaeresis = 0x0ff;
    static const int mitk::EventMapper::Key_unknown = 0xffff;
	static const int mitk::EventMapper::Key_none = Key_unknown;
*/
