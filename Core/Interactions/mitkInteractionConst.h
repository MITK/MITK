#ifndef MITKINTERACTCONST_H
#define MITKINTERACTCONST_H
//##Constants for EventMapping...
//##SideEffectId
#define DONOTHING 0
#define INITNEWOBJECT 5
#define INITEDITOBJECT 6
#define INITEDITGROUP 7
#define ADDPOINT 10
#define SNAPPING_ADDING 20
#define CHECKPOINT 30
#define CHECKIFLAST 31
#define FINISHOBJECT 40
#define FINISHGROUP 41
#define SEARCHOBJECT 50
#define SEARCHGROUP 51
#define SEARCHANOTHEROBJECT 52 //one object is selected and another object is to be added to selection
#define SELECTPICKEDOBJECT 60 // select the picked object and deselect others
#define SELECTANOTHEROBJECT 61 //adds the picked object to the selection
#define SELECTGROUP 62
#define SELECTALL 63
#define ADDSELECTEDTOGROUP 64
#define DESELECTOBJECT 70 //deselect picked from group
#define DESELECTALL 72
#define NEWPOINT 80

	//Type
#define None 0				// invalid event
#define Timer 1				// timer event
#define MouseButtonPress 2			// mouse button pressed
#define MouseButtonRelease 3			// mouse button released
#define MouseButtonDblClick 4		// mouse button double click
#define MouseMove 5				// mouse move
#define KeyPress 6				// key pressed
#define KeyRelease 7				// key released
#define FocusIn 8				// keyboard focus received
#define FocusOut 9				// keyboard focus lost
#define Enter 10				// mouse enters widget
#define Leave 11				// mouse leaves widget
#define Paint 12				// paint widget
#define Move 13				// move widget
#define Resize 14				// resize widget
#define Create 15				// after object creation
#define Destroy 16				// during object destruction
#define Show 17				// widget is shown
#define Hide 18				// widget is hidden
#define Close 19				// request to close widget
#define Quit 20				// request to quit application
#define Reparent 21				// widget has been reparented
#define ShowMinimized 22		       	// widget is shown minimized
#define ShowNormal 23	       		// widget is shown normal
#define WindowActivate 24	       		// window was activated
#define WindowDeactivate 25	       		// window was deactivated
#define ShowToParent 26	       		// widget is shown to parent
#define HideToParent 27	       		// widget is hidden to parent
#define ShowMaximized 28		       	// widget is shown maximized
#define ShowFullScreen 29			// widget is shown full-screen
#define Accel 30				// accelerator event
#define Wheel 31				// wheel event
#define AccelAvailable 32			// accelerator available event
#define CaptionChange 33			// caption changed
#define IconChange 34			// icon changed
#define ParentFontChange 35			// parent font changed
#define ApplicationFontChange 36		// application font changed
#define ParentPaletteChange 37		// parent palette changed
#define ApplicationPaletteChange 38		// application palette changed
#define PaletteChange 39			// widget palette changed
#define Clipboard 40				// internal clipboard event
#define Speech 42				// reserved for speech input
#define SockAct 50				// socket activation
#define AccelOverride 51			// accelerator override event
#define DeferredDelete 52			// deferred delete event
#define DragEnter 60				// drag moves into widget
#define DragMove 61				// drag moves in widget
#define DragLeave 62				// drag leaves or is cancelled
#define Drop 63				// actual drop
#define DragResponse 64			// drag accepted/rejected
#define ChildInserted 70			// new child widget
#define ChildRemoved 71			// deleted child widget
#define LayoutHint 72			// child min/max size changed
#define ShowWindowRequest 73			// widget's window should be mapped
#define ActivateControl 80			// ActiveX activation
#define DeactivateControl 81			// ActiveX deactivation
#define ContextMenu 82			// context popup menu
#define IMStart 83				// input method composition start
#define IMCompose 84				// input method composition
#define IMEnd 85				// input method composition end
#define Accessibility 86			// accessibility information is requested
#define TabletMove 87			// Wacom tablet event
#define LocaleChange 88			// the system locale changed
#define LanguageChange 89			// the application language changed
#define LayoutDirectionChange 90		// the layout direction changed
#define Style 91				// internal style event
#define TabletPress 92			// tablet press
#define TabletRelease 93			// tablet release
#define User 1000				// first user event id
#define MaxUser 65535		

	//##ButtonState
	// mouse/keyboard state values
#define NoButton        0x0000
#define LeftButton      0x0001
#define RightButton     0x0002
#define MidButton       0x0004
#define MouseButtonMask 0x0007
#define ShiftButton     0x0100
#define ControlButton   0x0200
#define AltButton       0x0400
#define MetaButton		0x0800
#define KeyButtonMask   0x0f00
#define Keypad          0x4000

	//##Key
#define Key_Escape 0x1000            // misc keys
#define Key_Tab 0x1001
#define Key_Backtab 0x1002 
#define Key_BackTab 0x1002 //= Key_Backtab
#define Key_Backspace 0x1003 
#define Key_BackSpace 0x1003 //= Key_Backspace
#define Key_Return 0x1004
#define Key_Enter 0x1005
#define Key_Insert 0x1006
#define Key_Delete 0x1007
#define Key_Pause 0x1008
#define Key_Print 0x1009
#define Key_SysReq 0x100a
#define Key_Home 0x1010              // cursor movement
#define Key_End 0x1011
#define Key_Left 0x1012
#define Key_Up 0x1013
#define Key_Right 0x1014
#define Key_Down 0x1015
#define Key_Prior 0x1016 
#define Key_PageUp Key_Prior
#define Key_Next 0x1017 
#define Key_PageDown Key_Next
#define Key_Shift 0x1020             // modifiers
#define Key_Control 0x1021
#define Key_Meta 0x1022
#define Key_Alt 0x1023
#define Key_CapsLock 0x1024
#define Key_NumLock 0x1025
#define Key_ScrollLock 0x1026
#define Key_F1 0x1030                // function keys
#define Key_F2 0x1031
#define Key_F3 0x1032
#define Key_F4 0x1033
#define Key_F5 0x1034
#define Key_F6 0x1035
#define Key_F7 0x1036
#define Key_F8 0x1037
#define Key_F9 0x1038
#define Key_F10 0x1039
#define Key_F11 0x103a
#define Key_F12 0x103b
#define Key_F13 0x103c
#define Key_F14 0x103d
#define Key_F15 0x103e
#define Key_F16 0x103f
#define Key_F17 0x1040
#define Key_F18 0x1041
#define Key_F19 0x1042
#define Key_F20 0x1043
#define Key_F21 0x1044
#define Key_F22 0x1045
#define Key_F23 0x1046
#define Key_F24 0x1047
#define Key_F25 0x1048               // F25 .. F35 only on X11
#define Key_F26 0x1049
#define Key_F27 0x104a
#define Key_F28 0x104b
#define Key_F29 0x104c
#define Key_F30 0x104d
#define Key_F31 0x104e
#define Key_F32 0x104f
#define Key_F33 0x1050
#define Key_F34 0x1051
#define Key_F35 0x1052
#define Key_Super_L 0x1053           // extra keys
#define Key_Super_R 0x1054
#define Key_Menu 0x1055
#define Key_Hyper_L 0x1056
#define Key_Hyper_R 0x1057
#define Key_Help 0x1058
        // International input method support (X keycode - 0xEE00)
        // Only interesting if you are writing your own input method
#define Key_Muhenkan  0x1122  // Cancel Conversion
#define Key_Henkan  0x1123  // Start/Stop Conversion
#define Key_Hiragana_Katakana 0x1127  // Hiragana/Katakana toggle
#define Key_Zenkaku_Hankaku 0x112A  // Zenkaku/Hankaku toggle
#define Key_Space 0x20               // 7 bit printable ASCII
#define Key_Any Key_Space
#define Key_Exclam 0x21
#define Key_QuoteDbl 0x22
#define Key_NumberSign 0x23
#define Key_Dollar 0x24
#define Key_Percent 0x25
#define Key_Ampersand 0x26
#define Key_Apostrophe 0x27
#define Key_ParenLeft 0x28
#define Key_ParenRight 0x29
#define Key_Asterisk 0x2a
#define Key_Plus 0x2b
#define Key_Comma 0x2c
#define Key_Minus 0x2d
#define Key_Period 0x2e
#define Key_Slash 0x2f
#define Key_0 0x30
#define Key_1 0x31
#define Key_2 0x32
#define Key_3 0x33
#define Key_4 0x34
#define Key_5 0x35
#define Key_6 0x36
#define Key_7 0x37
#define Key_8 0x38
#define Key_9 0x39
#define Key_Colon 0x3a
#define Key_Semicolon 0x3b
#define Key_Less 0x3c
#define Key_Equal 0x3d
#define Key_Greater 0x3e
#define Key_Question 0x3f
#define Key_At 0x40
#define Key_A 0x41
#define Key_B 0x42
#define Key_C 0x43
#define Key_D 0x44
#define Key_E 0x45
#define Key_F 0x46
#define Key_G 0x47
#define Key_H 0x48
#define Key_I 0x49
#define Key_J 0x4a
#define Key_K 0x4b
#define Key_L 0x4c
#define Key_M 0x4d
#define Key_N 0x4e
#define Key_O 0x4f
#define Key_P 0x50
#define Key_Q 0x51
#define Key_R 0x52
#define Key_S 0x53
#define Key_T 0x54
#define Key_U 0x55
#define Key_V 0x56
#define Key_W 0x57
#define Key_X 0x58
#define Key_Y 0x59
#define Key_Z 0x5a
#define Key_BracketLeft 0x5b
#define Key_Backslash 0x5c
#define Key_BracketRight 0x5d
#define Key_AsciiCircum 0x5e
#define Key_Underscore 0x5f
#define Key_QuoteLeft 0x60
#define Key_BraceLeft 0x7b
#define Key_Bar 0x7c
#define Key_BraceRight 0x7d
#define Key_AsciiTilde 0x7e
#define Key_nobreakspace 0x0a0
#define Key_exclamdown 0x0a1
#define Key_cent 0x0a2
#define Key_sterling 0x0a3
#define Key_currency 0x0a4
#define Key_yen 0x0a5
#define Key_brokenbar 0x0a6
#define Key_section 0x0a7
#define Key_diaeresis 0x0a8
#define Key_copyright 0x0a9
#define Key_ordfeminine 0x0aa
#define Key_guillemotleft 0x0ab      // left angle quotation mark
#define Key_notsign 0x0ac
#define Key_hyphen 0x0ad
#define Key_registered 0x0ae
#define Key_macron 0x0af
#define Key_degree 0x0b0
#define Key_plusminus 0x0b1
#define Key_twosuperior 0x0b2
#define Key_threesuperior 0x0b3
#define Key_acute 0x0b4
#define Key_mu 0x0b5
#define Key_paragraph 0x0b6
#define Key_periodcentered 0x0b7
#define Key_cedilla 0x0b8
#define Key_onesuperior 0x0b9
#define Key_masculine 0x0ba
#define Key_guillemotright 0x0bb     // right angle quotation mark
#define Key_onequarter 0x0bc
#define Key_onehalf 0x0bd
#define Key_threequarters 0x0be
#define Key_questiondown 0x0bf
#define Key_Agrave 0x0c0
#define Key_Aacute 0x0c1
#define Key_Acircumflex 0x0c2
#define Key_Atilde 0x0c3
#define Key_Adiaeresis 0x0c4
#define Key_Aring 0x0c5
#define Key_AE 0x0c6
#define Key_Ccedilla 0x0c7
#define Key_Egrave 0x0c8
#define Key_Eacute 0x0c9
#define Key_Ecircumflex 0x0ca
#define Key_Ediaeresis 0x0cb
#define Key_Igrave 0x0cc
#define Key_Iacute 0x0cd
#define Key_Icircumflex 0x0ce
#define Key_Idiaeresis 0x0cf
#define Key_ETH 0x0d0
#define Key_Ntilde 0x0d1
#define Key_Ograve 0x0d2
#define Key_Oacute 0x0d3
#define Key_Ocircumflex 0x0d4
#define Key_Otilde 0x0d5
#define Key_Odiaeresis 0x0d6
#define Key_multiply 0x0d7
#define Key_Ooblique 0x0d8
#define Key_Ugrave 0x0d9
#define Key_Uacute 0x0da
#define Key_Ucircumflex 0x0db
#define Key_Udiaeresis 0x0dc
#define Key_Yacute 0x0dd
#define Key_THORN 0x0de
#define Key_ssharp 0x0df
#define Key_agrave 0x0e0
#define Key_aacute 0x0e1
#define Key_acircumflex 0x0e2
#define Key_atilde 0x0e3
#define Key_adiaeresis 0x0e4
#define Key_aring 0x0e5
#define Key_ae 0x0e6
#define Key_ccedilla 0x0e7
#define Key_egrave 0x0e8
#define Key_eacute 0x0e9
#define Key_ecircumflex 0x0ea
#define Key_ediaeresis 0x0eb
#define Key_igrave 0x0ec
#define Key_iacute 0x0ed
#define Key_icircumflex 0x0ee
#define Key_idiaeresis 0x0ef
#define Key_eth 0x0f0
#define Key_ntilde 0x0f1
#define Key_ograve 0x0f2
#define Key_oacute 0x0f3
#define Key_ocircumflex 0x0f4
#define Key_otilde 0x0f5
#define Key_odiaeresis 0x0f6
#define Key_division 0x0f7
#define Key_oslash 0x0f8
#define Key_ugrave 0x0f9
#define Key_uacute 0x0fa
#define Key_ucircumflex 0x0fb
#define Key_udiaeresis 0x0fc
#define Key_yacute 0x0fd
#define Key_thorn 0x0fe
#define Key_ydiaeresis 0x0ff
#define Key_unknown 0xffff 
#define Key_none 0xffff//= Key_unknown



/*
//
//UndoMechanism:
//constants for UndoMechanism;
*/
//different UndoModels:
#define LIMITEDLINEARUNDO 10
#define TREEUNDO 20

#endif //ifndef MITKINTERACTCONST_H
