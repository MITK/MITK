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



struct StringIntArrayStruct {//for mapping string from xml-file to int
	const char * text;
	int id;
};



namespace mitk {



//##ModelId=3E5A390401F2

class EventMapper : public QXmlDefaultHandler

{

  public:

    //##ModelId=3E5B349600CB

    static void SetStateMachine(StateMachine* stateMachine);



    //##ModelId=3E5B34CF0041

    static bool MapEvent(Event* event);



    //##ModelId=3E5B35140072

	static bool LoadBehavior(std::string fileName);



    //##ModelId=3E788FC00308

	bool startElement( const QString&, const QString&, const QString& qName, const QXmlAttributes& atts );

    //##ModelId=3E7B20EE01F5

    std::string GetStyleName();





  private:

    //##ModelId=3E5B33F303CA

	static EventDescriptionVec m_EventDescriptions;



    //##ModelId=3E5B343701F1

    static StateMachine *m_StateMachine;



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



public:

	//##Constants for EventMapping... In StateMachines do: case EventMapper::KeyPress

	//##SideEffectId


/*	static const int DONOTHING;
	static const int INITNEWOBJECT;
	static const int INITEDITOBJECT = 6;
	static const int INITEDITGROUP = 7;
	static const int ADDPOINT = 10;
	static const int SNAPPING_ADDING = 20;
	static const int CHECKPOINT = 30;
	static const int CHECKIFLAST = 31;
	static const int FINISHOBJECT = 40;
	static const int FINISHGROUP = 41;
	static const int SEARCHOBJECT = 50;
	static const int SEARCHGROUP = 51;
	static const int SEARCHANOTHEROBJECT = 52; //one object is selected and another object is to be added to selection
	static const int SELECTPICKEDOBJECT = 60; // select the picked object and deselect others
	static const int SELECTANOTHEROBJECT = 61; //adds the picked object to the selection
	static const int SELECTGROUP = 62;
    static const int SELECTALL = 63;
	static const int ADDSELECTEDTOGROUP = 63;
	static const int DESELECTOBJECT = 70; //deselect picked from group
	static const int DESELECTALL = 72;



	//Type

	static const int None = 0;				// invalid event
	static const int Timer = 1;				// timer event
	static const int MouseButtonPress = 2;			// mouse button pressed
	static const int MouseButtonRelease = 3;			// mouse button released
	static const int MouseButtonDblClick = 4;		// mouse button double click
	static const int MouseMove = 5;				// mouse move
	static const int KeyPress = 6;				// key pressed
	static const int KeyRelease = 7;				// key released
	static const int FocusIn = 8;				// keyboard focus received
	static const int FocusOut = 9;				// keyboard focus lost
	static const int Enter = 10;				// mouse enters widget
	static const int Leave = 11;				// mouse leaves widget
	static const int Paint = 12;				// paint widget
	static const int Move = 13;				// move widget
	static const int Resize = 14;				// resize widget
	static const int Create = 15;				// after object creation
	static const int Destroy = 16;				// during object destruction
	static const int Show = 17;				// widget is shown
	static const int Hide = 18;				// widget is hidden
	static const int Close = 19;				// request to close widget
	static const int Quit = 20;				// request to quit application
	static const int Reparent = 21;				// widget has been reparented
	static const int ShowMinimized = 22;		       	// widget is shown minimized
	static const int ShowNormal = 23;	       		// widget is shown normal
	static const int WindowActivate = 24;	       		// window was activated
	static const int WindowDeactivate = 25;	       		// window was deactivated
	static const int ShowToParent = 26;	       		// widget is shown to parent
	static const int HideToParent = 27;	       		// widget is hidden to parent
	static const int ShowMaximized = 28;		       	// widget is shown maximized
	static const int ShowFullScreen = 29;			// widget is shown full-screen
	static const int Accel = 30;				// accelerator event
	static const int Wheel = 31;				// wheel event
	static const int AccelAvailable = 32;			// accelerator available event
	static const int CaptionChange = 33;			// caption changed
	static const int IconChange = 34;			// icon changed
	static const int ParentFontChange = 35;			// parent font changed
	static const int ApplicationFontChange = 36;		// application font changed
	static const int ParentPaletteChange = 37;		// parent palette changed
	static const int ApplicationPaletteChange = 38;		// application palette changed
	static const int PaletteChange = 39;			// widget palette changed
	static const int Clipboard = 40;				// internal clipboard event
	static const int Speech = 42;				// reserved for speech input
	static const int SockAct = 50;				// socket activation
	static const int AccelOverride = 51;			// accelerator override event
	static const int DeferredDelete = 52;			// deferred delete event
	static const int DragEnter = 60;				// drag moves into widget
	static const int DragMove = 61;				// drag moves in widget
	static const int DragLeave = 62;				// drag leaves or is cancelled
	static const int Drop = 63;				// actual drop
	static const int DragResponse = 64;			// drag accepted/rejected
	static const int ChildInserted = 70;			// new child widget
	static const int ChildRemoved = 71;			// deleted child widget
	static const int LayoutHint = 72;			// child min/max size changed
	static const int ShowWindowRequest = 73;			// widget's window should be mapped
	static const int ActivateControl = 80;			// ActiveX activation
	static const int DeactivateControl = 81;			// ActiveX deactivation
	static const int ContextMenu = 82;			// context popup menu
	static const int IMStart = 83;				// input method composition start
	static const int IMCompose = 84;				// input method composition
	static const int IMEnd = 85;				// input method composition end
	static const int Accessibility = 86;			// accessibility information is requested
	static const int TabletMove = 87;			// Wacom tablet event
	static const int LocaleChange = 88;			// the system locale changed
	static const int LanguageChange = 89;			// the application language changed
	static const int LayoutDirectionChange = 90;		// the layout direction changed
	static const int Style = 91;				// internal style event
	static const int TabletPress = 92;			// tablet press
	static const int TabletRelease = 93;			// tablet release
	static const int User = 1000;				// first user event id
	static const int MaxUser = 65535;		



	//##ButtonState

	// mouse/keyboard state values
    static const int NoButton        = 0x0000;
    static const int LeftButton      = 0x0001;
    static const int RightButton     = 0x0002;
    static const int MidButton       = 0x0004;
    static const int MouseButtonMask = 0x0007;
    static const int ShiftButton     = 0x0008;
    static const int ControlButton   = 0x0010;
    static const int AltButton       = 0x0020;
    static const int KeyButtonMask   = 0x0038;
    static const int Keypad          = 0x4000;

	//##Key
    static const int Key_Escape = 0x1000;            // misc keys
    static const int Key_Tab = 0x1001;
    static const int Key_Backtab = 0x1002; 
	static const int Key_BackTab = Key_Backtab;
    static const int Key_Backspace = 0x1003; 
	static const int Key_BackSpace = Key_Backspace;
    static const int Key_Return = 0x1004;
    static const int Key_Enter = 0x1005;
    static const int Key_Insert = 0x1006;
    static const int Key_Delete = 0x1007;
    static const int Key_Pause = 0x1008;
    static const int Key_Print = 0x1009;
    static const int Key_SysReq = 0x100a;
    static const int Key_Home = 0x1010;              // cursor movement
    static const int Key_End = 0x1011;
    static const int Key_Left = 0x1012;
    static const int Key_Up = 0x1013;
    static const int Key_Right = 0x1014;
    static const int Key_Down = 0x1015;
    static const int Key_Prior = 0x1016; 
	static const int Key_PageUp = Key_Prior;
    static const int Key_Next = 0x1017; 
	static const int Key_PageDown = Key_Next;
    static const int Key_Shift = 0x1020;             // modifiers
    static const int Key_Control = 0x1021;
    static const int Key_Meta = 0x1022;
    static const int Key_Alt = 0x1023;
    static const int Key_CapsLock = 0x1024;
    static const int Key_NumLock = 0x1025;
    static const int Key_ScrollLock = 0x1026;
    static const int Key_F1 = 0x1030;                // function keys
    static const int Key_F2 = 0x1031;
    static const int Key_F3 = 0x1032;
    static const int Key_F4 = 0x1033;
    static const int Key_F5 = 0x1034;
    static const int Key_F6 = 0x1035;
    static const int Key_F7 = 0x1036;
    static const int Key_F8 = 0x1037;
    static const int Key_F9 = 0x1038;
    static const int Key_F10 = 0x1039;
    static const int Key_F11 = 0x103a;
    static const int Key_F12 = 0x103b;
    static const int Key_F13 = 0x103c;
    static const int Key_F14 = 0x103d;
    static const int Key_F15 = 0x103e;
    static const int Key_F16 = 0x103f;
    static const int Key_F17 = 0x1040;
    static const int Key_F18 = 0x1041;
    static const int Key_F19 = 0x1042;
    static const int Key_F20 = 0x1043;
    static const int Key_F21 = 0x1044;
    static const int Key_F22 = 0x1045;
    static const int Key_F23 = 0x1046;
    static const int Key_F24 = 0x1047;
    static const int Key_F25 = 0x1048;               // F25 .. F35 only on X11
    static const int Key_F26 = 0x1049;
    static const int Key_F27 = 0x104a;
    static const int Key_F28 = 0x104b;
    static const int Key_F29 = 0x104c;
    static const int Key_F30 = 0x104d;
    static const int Key_F31 = 0x104e;
    static const int Key_F32 = 0x104f;
    static const int Key_F33 = 0x1050;
    static const int Key_F34 = 0x1051;
    static const int Key_F35 = 0x1052;
    static const int Key_Super_L = 0x1053;           // extra keys
    static const int Key_Super_R = 0x1054;
    static const int Key_Menu = 0x1055;
    static const int Key_Hyper_L = 0x1056;
    static const int Key_Hyper_R = 0x1057;
    static const int Key_Help = 0x1058;
        // International input method support (X keycode - 0xEE00)
        // Only interesting if you are writing your own input method
    static const int Key_Muhenkan =  0x1122;  // Cancel Conversion
    static const int Key_Henkan =  0x1123;  // Start/Stop Conversion
    static const int Key_Hiragana_Katakana = 0x1127;  // Hiragana/Katakana toggle
    static const int Key_Zenkaku_Hankaku = 0x112A;  // Zenkaku/Hankaku toggle
    static const int Key_Space = 0x20;               // 7 bit printable ASCII
    static const int Key_Any = Key_Space;
    static const int Key_Exclam = 0x21;
    static const int Key_QuoteDbl = 0x22;
    static const int Key_NumberSign = 0x23;
    static const int Key_Dollar = 0x24;
    static const int Key_Percent = 0x25;
    static const int Key_Ampersand = 0x26;
    static const int Key_Apostrophe = 0x27;
    static const int Key_ParenLeft = 0x28;
    static const int Key_ParenRight = 0x29;
    static const int Key_Asterisk = 0x2a;
    static const int Key_Plus = 0x2b;
    static const int Key_Comma = 0x2c;
    static const int Key_Minus = 0x2d;
    static const int Key_Period = 0x2e;
    static const int Key_Slash = 0x2f;
    static const int Key_0 = 0x30;
    static const int Key_1 = 0x31;
    static const int Key_2 = 0x32;
    static const int Key_3 = 0x33;
    static const int Key_4 = 0x34;
    static const int Key_5 = 0x35;
    static const int Key_6 = 0x36;
    static const int Key_7 = 0x37;
    static const int Key_8 = 0x38;
    static const int Key_9 = 0x39;
    static const int Key_Colon = 0x3a;
    static const int Key_Semicolon = 0x3b;
    static const int Key_Less = 0x3c;
    static const int Key_Equal = 0x3d;
    static const int Key_Greater = 0x3e;
    static const int Key_Question = 0x3f;
    static const int Key_At = 0x40;
    static const int Key_A = 0x41;
    static const int Key_B = 0x42;
    static const int Key_C = 0x43;
    static const int Key_D = 0x44;
    static const int Key_E = 0x45;
    static const int Key_F = 0x46;
    static const int Key_G = 0x47;
    static const int Key_H = 0x48;
    static const int Key_I = 0x49;
    static const int Key_J = 0x4a;
    static const int Key_K = 0x4b;
    static const int Key_L = 0x4c;
    static const int Key_M = 0x4d;
    static const int Key_N = 0x4e;
    static const int Key_O = 0x4f;
    static const int Key_P = 0x50;
    static const int Key_Q = 0x51;
    static const int Key_R = 0x52;
    static const int Key_S = 0x53;
    static const int Key_T = 0x54;
    static const int Key_U = 0x55;
    static const int Key_V = 0x56;
    static const int Key_W = 0x57;
    static const int Key_X = 0x58;
    static const int Key_Y = 0x59;
    static const int Key_Z = 0x5a;
    static const int Key_BracketLeft = 0x5b;
    static const int Key_Backslash = 0x5c;
    static const int Key_BracketRight = 0x5d;
    static const int Key_AsciiCircum = 0x5e;
    static const int Key_Underscore = 0x5f;
    static const int Key_QuoteLeft = 0x60;
    static const int Key_BraceLeft = 0x7b;
    static const int Key_Bar = 0x7c;
    static const int Key_BraceRight = 0x7d;
    static const int Key_AsciiTilde = 0x7e;
        // Latin 1 codes adapted from X: keysymdef.h;v 1.21 94/08/28 16:17:06
    static const int Key_nobreakspace = 0x0a0;
    static const int Key_exclamdown = 0x0a1;
    static const int Key_cent = 0x0a2;
    static const int Key_sterling = 0x0a3;
    static const int Key_currency = 0x0a4;
    static const int Key_yen = 0x0a5;
    static const int Key_brokenbar = 0x0a6;
    static const int Key_section = 0x0a7;
    static const int Key_diaeresis = 0x0a8;
    static const int Key_copyright = 0x0a9;
    static const int Key_ordfeminine = 0x0aa;
    static const int Key_guillemotleft = 0x0ab;      // left angle quotation mark
    static const int Key_notsign = 0x0ac;
    static const int Key_hyphen = 0x0ad;
    static const int Key_registered = 0x0ae;
    static const int Key_macron = 0x0af;
    static const int Key_degree = 0x0b0;
    static const int Key_plusminus = 0x0b1;
    static const int Key_twosuperior = 0x0b2;
    static const int Key_threesuperior = 0x0b3;
    static const int Key_acute = 0x0b4;
    static const int Key_mu = 0x0b5;
    static const int Key_paragraph = 0x0b6;
    static const int Key_periodcentered = 0x0b7;
    static const int Key_cedilla = 0x0b8;
    static const int Key_onesuperior = 0x0b9;
    static const int Key_masculine = 0x0ba;
    static const int Key_guillemotright = 0x0bb;     // right angle quotation mark
    static const int Key_onequarter = 0x0bc;
    static const int Key_onehalf = 0x0bd;
    static const int Key_threequarters = 0x0be;
    static const int Key_questiondown = 0x0bf;
    static const int Key_Agrave = 0x0c0;
    static const int Key_Aacute = 0x0c1;
    static const int Key_Acircumflex = 0x0c2;
    static const int Key_Atilde = 0x0c3;
    static const int Key_Adiaeresis = 0x0c4;
    static const int Key_Aring = 0x0c5;
    static const int Key_AE = 0x0c6;
    static const int Key_Ccedilla = 0x0c7;
    static const int Key_Egrave = 0x0c8;
    static const int Key_Eacute = 0x0c9;
    static const int Key_Ecircumflex = 0x0ca;
    static const int Key_Ediaeresis = 0x0cb;
    static const int Key_Igrave = 0x0cc;
    static const int Key_Iacute = 0x0cd;
    static const int Key_Icircumflex = 0x0ce;
    static const int Key_Idiaeresis = 0x0cf;
    static const int Key_ETH = 0x0d0;
    static const int Key_Ntilde = 0x0d1;
    static const int Key_Ograve = 0x0d2;
    static const int Key_Oacute = 0x0d3;
    static const int Key_Ocircumflex = 0x0d4;
    static const int Key_Otilde = 0x0d5;
    static const int Key_Odiaeresis = 0x0d6;
    static const int Key_multiply = 0x0d7;
    static const int Key_Ooblique = 0x0d8;
    static const int Key_Ugrave = 0x0d9;
    static const int Key_Uacute = 0x0da;
    static const int Key_Ucircumflex = 0x0db;
    static const int Key_Udiaeresis = 0x0dc;
    static const int Key_Yacute = 0x0dd;
    static const int Key_THORN = 0x0de;
    static const int Key_ssharp = 0x0df;
    static const int Key_agrave = 0x0e0;
    static const int Key_aacute = 0x0e1;
    static const int Key_acircumflex = 0x0e2;
    static const int Key_atilde = 0x0e3;
    static const int Key_adiaeresis = 0x0e4;
    static const int Key_aring = 0x0e5;
    static const int Key_ae = 0x0e6;
    static const int Key_ccedilla = 0x0e7;
    static const int Key_egrave = 0x0e8;
    static const int Key_eacute = 0x0e9;
    static const int Key_ecircumflex = 0x0ea;
    static const int Key_ediaeresis = 0x0eb;
    static const int Key_igrave = 0x0ec;
    static const int Key_iacute = 0x0ed;
    static const int Key_icircumflex = 0x0ee;
    static const int Key_idiaeresis = 0x0ef;
    static const int Key_eth = 0x0f0;
    static const int Key_ntilde = 0x0f1;
    static const int Key_ograve = 0x0f2;
    static const int Key_oacute = 0x0f3;
    static const int Key_ocircumflex = 0x0f4;
    static const int Key_otilde = 0x0f5;
    static const int Key_odiaeresis = 0x0f6;
    static const int Key_division = 0x0f7;
    static const int Key_oslash = 0x0f8;
    static const int Key_ugrave = 0x0f9;
    static const int Key_uacute = 0x0fa;
    static const int Key_ucircumflex = 0x0fb;
    static const int Key_udiaeresis = 0x0fc;
    static const int Key_yacute = 0x0fd;
    static const int Key_thorn = 0x0fe;
    static const int Key_ydiaeresis = 0x0ff;
    static const int Key_unknown = 0xffff; 
	static const int Key_none = Key_unknown;
*/

};



} // namespace mitk




//const int mitk::EventMapper::DONOTHING = 0;
//const int mitk::EventMapper::INITNEWOBJECT = 5;




#endif /* EVENTMAPPER_H_HEADER_INCLUDED_C187864A */

