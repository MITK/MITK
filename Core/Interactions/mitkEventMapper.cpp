#include "EventMapper.h"
#include "PositionEvent.h"
#include <string>
#include "FocusManager.h"

//XML Event
//##ModelId=3E788FC000E5
const std::string mitk::EventMapper::STYLE = "STYLE";
//##ModelId=3E788FC0025C
const std::string mitk::EventMapper::NAME = "NAME";
//##ModelId=3E788FC002AA
const std::string mitk::EventMapper::ID = "ID";
//##ModelId=3E77572901E9
const std::string mitk::EventMapper::TYPE = "TYPE";
//##ModelId=3E7757290256
const std::string mitk::EventMapper::BUTTON = "BUTTON";
//##ModelId=3E8B08FA01AA
const std::string mitk::EventMapper::BUTTONSTATE = "BUTTONSTATE";
//##ModelId=3E77572902C4
const std::string mitk::EventMapper::KEY = "KEY";

//for Linker ->no undefined reference
mitk::StateMachine *mitk::EventMapper::m_GlobalStateMachine;
EventDescriptionVec mitk::EventMapper::m_EventDescriptions;
mitk::StateEvent mitk::EventMapper::m_StateEvent;
std::string mitk::EventMapper::m_StyleName;


//array with string to key for mapping string from xml-file to int
StringIntArrayStruct mitkConstType[] = {
	{"None",					0},		// invalid event
	{"Timer",					1},		// timer event
	{"MouseButtonPress" ,		2},		// mouse button pressed
	{"MouseButtonRelease" ,		3},		// mouse button released
	{"MouseButtonDblClick" ,	4},		// mouse button double click
	{"MouseMove" ,				5},		// mouse move
	{"KeyPress" ,				6},		// key pressed
	{"KeyRelease" ,				7},		// key released
	{"FocusIn" ,				8},		// keyboard focus received
	{"FocusOut" ,				9},		// keyboard focus lost
	{"Enter" ,					10},	// mouse enters widget
	{"Leave" ,					11},	// mouse leaves widget
	{"Paint" ,					12},	// paint widget
	{"Move" ,					13},	// move widget
	{"Resize" ,					14},	// resize widget
	{"Create" ,					15},	// after object creation
	{"Destroy" ,				16},	// during object destruction
	{"Show" ,					17},	// widget is shown
	{"Hide" ,					18},	// widget is hidden
	{"Close" ,					19},	// request to close widget
	{"Quit" ,					20},	// request to quit application
	{"Reparent" ,				21},	// widget has been reparented
	{"ShowMinimized" ,			22},	// widget is shown minimized
	{"ShowNormal" ,				23},	// widget is shown normal
	{"WindowActivate" ,			24},	// window was activated
	{"WindowDeactivate" ,		25},	// window was deactivated
	{"ShowToParent" ,			26},	// widget is shown to parent
	{"HideToParent" ,			27},	// widget is hidden to parent
	{"ShowMaximized" ,			28},	// widget is shown maximized
	{"ShowFullScreen" ,			29},	// widget is shown full-screen
	{"Accel" ,					30},	// accelerator event
	{"Wheel" ,					31},	// wheel event
	{"AccelAvailable" ,			32},	// accelerator available event
	{"CaptionChange" ,			33},	// caption changed
	{"IconChange" ,				34},	// icon changed
	{"ParentFontChange" ,		35},	// parent font changed
	{"ApplicationFontChange" ,	36},// application font changed
	{"ParentPaletteChange" ,	37},	// parent palette changed
	{"ApplicationPaletteChange" , 38},// application palette changed
	{"PaletteChange" ,			39},	// widget palette changed
	{"Clipboard" ,				40},	// internal clipboard event
	{"Speech" ,					42},	// reserved for speech input
	{"SockAct" ,				50},	// socket activation
	{"AccelOverride" ,			51},	// accelerator override event
	{"DeferredDelete" ,			52},	// deferred delete event
	{"DragEnter" ,				60},	// drag moves into widget
	{"DragMove" ,				61},	// drag moves in widget
	{"DragLeave" ,				62},	// drag leaves or is cancelled
	{"Drop" ,					63},	// actual drop
	{"DragResponse" ,			64},	// drag accepted/rejected
	{"ChildInserted" ,			70},	// new child widget
	{"ChildRemoved" ,			71},	// deleted child widget
	{"LayoutHint" ,				72},	// child min/max size changed
	{"ShowWindowRequest" ,		73},	// widget's window should be mapped
	{"ActivateControl" ,		80},	// ActiveX activation
	{"DeactivateControl" ,		81},	// ActiveX deactivation
	{"ContextMenu" ,			82},	// context popup menu
	{"IMStart" ,				83},	// input method composition start
	{"IMCompose" ,				84},	// input method composition
	{"IMEnd" ,					85},	// input method composition end
	{"Accessibility" ,			86},	// accessibility information is requested
	{"TabletMove" ,				87},	// Wacom tablet event
	{"LocaleChange" ,			88},	// the system locale changed
	{"LanguageChange" ,			89},	// the application language changed
	{"LayoutDirectionChange" ,	90},	// the layout direction changed
	{"Style" ,					91},	// internal style event
	{"TabletPress" ,			92},	// tablet press
	{"TabletRelease" ,			93},	// tablet release
	{"User" ,					1000},	// first user event id
	{"MaxUser" ,				65535}	// last user event id
};
//array with string to key for mapping string from xml-file to int
StringIntArrayStruct mitkConstArray[] = {
//ButtonState
        {"NoButton" , 0x0000},
        {"LeftButton" , 0x0001},
        {"RightButton" , 0x0002},
        {"MidButton" , 0x0004},
        {"MouseButtonMask" , 0x0007},
        {"ShiftButton" , 0x0008},
        {"ControlButton" , 0x0010},
        {"AltButton" , 0x0020},
        {"KeyButtonMask" , 0x0038},
        {"Keypad" , 0x4000},

//Modifier
        {"SHIFT" , 0x00200000},
        {"CTRL" , 0x00400000},
        {"ALT" , 0x00800000},
        {"MODIFIER_MASK" , 0x00e00000},
        {"UNICODE_ACCEL" , 0x10000000},

        {"ASCII_ACCEL" , 0x10000000},

//Key
       {"Key_Escape", 0x1000},
       {"Key_Tab", 0x1001},
       {"Key_Backtab", 0x1002},
       {"Key_BackTab", 0x1002},
       {"Key_Backspace", 0x1003},
       {"Key_BackSpace", 0x1003},
       {"Key_Return", 0x1004},
       {"Key_Enter", 0x1005},
       {"Key_Insert", 0x1006},
       {"Key_Delete", 0x1007},
       {"Key_Pause", 0x1008},
       {"Key_Print", 0x1009},
       {"Key_SysReq", 0x100a},
		{"Key_Home", 0x1010},
       {"Key_End", 0x1011},
       {"Key_Left", 0x1012},
       {"Key_Up", 0x1013},
       {"Key_Right", 0x1014},
       {"Key_Down", 0x1015},
       {"Key_Prior", 0x1016},
       {"Key_PageUp", 0x1016},
       {"Key_Next", 0x1017},
       {"Key_PageDown", 0x1017},
       {"Key_Shift", 0x1020},
       {"Key_Control", 0x1021},
       {"Key_Meta", 0x1022},
       {"Key_Alt", 0x1023},
       {"Key_CapsLock", 0x1024},
       {"Key_NumLock", 0x1025},
       {"Key_ScrollLock", 0x1026},
       {"Key_F1", 0x1030},
       {"Key_F2", 0x1031},
       {"Key_F3", 0x1032},
       {"Key_F4", 0x1033},
       {"Key_F5", 0x1034},
       {"Key_F6", 0x1035},
       {"Key_F7", 0x1036},
       {"Key_F8", 0x1037},
       {"Key_F9", 0x1038},
       {"Key_F10", 0x1039},
       {"Key_F11", 0x103a},
       {"Key_F12", 0x103b},
       {"Key_F13", 0x103c},
       {"Key_F14", 0x103d},
       {"Key_F15", 0x103e},
       {"Key_F16", 0x103f},
       {"Key_F17", 0x1040},
       {"Key_F18", 0x1041},
       {"Key_F19", 0x1042},
       {"Key_F20", 0x1043},
       {"Key_F21", 0x1044},
       {"Key_F22", 0x1045},
       {"Key_F23", 0x1046},
       {"Key_F24", 0x1047},
       {"Key_F25", 0x1048},
       {"Key_F26", 0x1049},
       {"Key_F27", 0x104a},
       {"Key_F28", 0x104b},
       {"Key_F29", 0x104c},
       {"Key_F30", 0x104d},
       {"Key_F31", 0x104e},
       {"Key_F32", 0x104f},
       {"Key_F33", 0x1050},
       {"Key_F34", 0x1051},
       {"Key_F35", 0x1052},
       {"Key_Super_L", 0x1053},
       {"Key_Super_R", 0x1054},
       {"Key_Menu", 0x1055},
       {"Key_Hyper_L", 0x1056},
       {"Key_Hyper_R", 0x1057},
       {"Key_Help", 0x1058},
       {"Key_Muhenkan",  0x1122},
       {"Key_Henkan",  0x1123},
       {"Key_Hiragana_Katakana", 0x1127},
       {"Key_Zenkaku_Hankaku", 0x112A},
       {"Key_Space", 0x20},
       {"Key_Any", 0x20},
       {"Key_Exclam", 0x21},
       {"Key_QuoteDbl", 0x22},
       {"Key_NumberSign", 0x23},
       {"Key_Dollar", 0x24},
       {"Key_Percent", 0x25},
       {"Key_Ampersand", 0x26},
       {"Key_Apostrophe", 0x27},
       {"Key_ParenLeft", 0x28},
       {"Key_ParenRight", 0x29},
       {"Key_Asterisk", 0x2a},
       {"Key_Plus", 0x2b},
       {"Key_Comma", 0x2c},
       {"Key_Minus", 0x2d},
       {"Key_Period", 0x2e},
       {"Key_Slash", 0x2f},
       {"Key_0", 0x30},
       {"Key_1", 0x31},
       {"Key_2", 0x32},
       {"Key_3", 0x33},
       {"Key_4", 0x34},
       {"Key_5", 0x35},
       {"Key_6", 0x36},
       {"Key_7", 0x37},
       {"Key_8", 0x38},
       {"Key_9", 0x39},
       {"Key_Colon", 0x3a},
       {"Key_Semicolon", 0x3b},
       {"Key_Less", 0x3c},
       {"Key_Equal", 0x3d},
       {"Key_Greater", 0x3e},
       {"Key_Question", 0x3f},
       {"Key_At", 0x40},
       {"Key_A", 0x41},
       {"Key_B", 0x42},
       {"Key_C", 0x43},
       {"Key_D", 0x44},
       {"Key_E", 0x45},
       {"Key_F", 0x46},
       {"Key_G", 0x47},
       {"Key_H", 0x48},
       {"Key_I", 0x49},
       {"Key_J", 0x4a},
       {"Key_K", 0x4b},
       {"Key_L", 0x4c},
       {"Key_M", 0x4d},
       {"Key_N", 0x4e},
	   {"Key_O", 0x4f},
	   {"Key_P", 0x50},
       {"Key_Q", 0x51},
       {"Key_R", 0x52},
       {"Key_S", 0x53},
       {"Key_T", 0x54},
       {"Key_U", 0x55},
       {"Key_V", 0x56},
       {"Key_W", 0x57},
       {"Key_X", 0x58},
       {"Key_Y", 0x59},
       {"Key_Z", 0x5a},
       {"Key_BracketLeft", 0x5b},
       {"Key_Backslash", 0x5c},
       {"Key_BracketRight", 0x5d},
       {"Key_AsciiCircum", 0x5e},
       {"Key_Underscore", 0x5f},
       {"Key_QuoteLeft", 0x60},
       {"Key_BraceLeft", 0x7b},
       {"Key_Bar", 0x7c},
       {"Key_BraceRight", 0x7d},
       {"Key_AsciiTilde", 0x7e},
       {"Key_nobreakspace", 0x0a0},
       {"Key_exclamdown", 0x0a1},
       {"Key_cent", 0x0a2},
       {"Key_sterling", 0x0a3},
       {"Key_currency", 0x0a4},
       {"Key_yen", 0x0a5},
       {"Key_brokenbar", 0x0a6},
       {"Key_section", 0x0a7},
       {"Key_diaeresis", 0x0a8},
       {"Key_copyright", 0x0a9},
       {"Key_ordfeminine", 0x0aa},
       {"Key_guillemotleft", 0x0ab},
       {"Key_notsign", 0x0ac},
       {"Key_hyphen", 0x0ad},
       {"Key_registered", 0x0ae},
       {"Key_macron", 0x0af},
       {"Key_degree", 0x0b0},
       {"Key_plusminus", 0x0b1},
       {"Key_twosuperior", 0x0b2},
       {"Key_threesuperior", 0x0b3},
       {"Key_acute", 0x0b4},
       {"Key_mu", 0x0b5},
       {"Key_paragraph", 0x0b6},
       {"Key_periodcentered", 0x0b7},
       {"Key_cedilla", 0x0b8},
       {"Key_onesuperior", 0x0b9},
       {"Key_masculine", 0x0ba},
       {"Key_guillemotright", 0x0bb},
       {"Key_onequarter", 0x0bc},
       {"Key_onehalf", 0x0bd},
       {"Key_threequarters", 0x0be},
       {"Key_questiondown", 0x0bf},
       {"Key_Agrave", 0x0c0},
       {"Key_Aacute", 0x0c1},
       {"Key_Acircumflex", 0x0c2},
       {"Key_Atilde", 0x0c3},
       {"Key_Adiaeresis", 0x0c4},
       {"Key_Aring", 0x0c5},
       {"Key_AE", 0x0c6},
       {"Key_Ccedilla", 0x0c7},
       {"Key_Egrave", 0x0c8},
       {"Key_Eacute", 0x0c9},
       {"Key_Ecircumflex", 0x0ca},
       {"Key_Ediaeresis", 0x0cb},
       {"Key_Igrave", 0x0cc},
       {"Key_Iacute", 0x0cd},
       {"Key_Icircumflex", 0x0ce},
       {"Key_Idiaeresis", 0x0cf},
       {"Key_ETH", 0x0d0},
       {"Key_Ntilde", 0x0d1},
       {"Key_Ograve", 0x0d2},
       {"Key_Oacute", 0x0d3},
       {"Key_Ocircumflex", 0x0d4},
       {"Key_Otilde", 0x0d5},
       {"Key_Odiaeresis", 0x0d6},
       {"Key_multiply", 0x0d7},
       {"Key_Ooblique", 0x0d8},
       {"Key_Ugrave", 0x0d9},
       {"Key_Uacute", 0x0da},
       {"Key_Ucircumflex", 0x0db},
       {"Key_Udiaeresis", 0x0dc},
       {"Key_Yacute", 0x0dd},
       {"Key_THORN", 0x0de},
       {"Key_ssharp", 0x0df},
       {"Key_agrave", 0x0e0},
       {"Key_aacute", 0x0e1},
       {"Key_acircumflex", 0x0e2},
       {"Key_atilde", 0x0e3},
       {"Key_adiaeresis", 0x0e4},
       {"Key_aring", 0x0e5},
       {"Key_ae", 0x0e6},
       {"Key_ccedilla", 0x0e7},
       {"Key_egrave", 0x0e8},
       {"Key_eacute", 0x0e9},
       {"Key_ecircumflex", 0x0ea},
       {"Key_ediaeresis", 0x0eb},
       {"Key_igrave", 0x0ec},
       {"Key_iacute", 0x0ed},
       {"Key_icircumflex", 0x0ee},
       {"Key_idiaeresis", 0x0ef},
       {"Key_eth", 0x0f0},
       {"Key_ntilde", 0x0f1},
       {"Key_ograve", 0x0f2},
       {"Key_oacute", 0x0f3},
       {"Key_ocircumflex", 0x0f4},
       {"Key_otilde", 0x0f5},
       {"Key_odiaeresis", 0x0f6},
       {"Key_division", 0x0f7},
       {"Key_oslash", 0x0f8},
       {"Key_ugrave", 0x0f9},
       {"Key_uacute", 0x0fa},
       {"Key_ucircumflex", 0x0fb},
       {"Key_udiaeresis", 0x0fc},
       {"Key_yacute", 0x0fd},
       {"Key_thorn", 0x0fe},
       {"Key_ydiaeresis", 0x0ff},
	   {"Key_unknown", 0xffff},
	   {"Key_none", 0xffff}
};




//##ModelId=3E5B349600CB
//##Documentation
//## set the global StateMachine. If temporaryly changed, 
//## then copy the old statemachine with GetStateMachine()
void mitk::EventMapper::SetGlobalStateMachine(StateMachine *stateMachine)
{
	m_GlobalStateMachine = stateMachine;
}

//##ModelId=3EDAFB81012E
mitk::StateMachine* mitk::EventMapper::GetGlobalStateMachine()
{
	return m_GlobalStateMachine;
}

//##ModelId=3E5B34CF0041
//##Documentation
//## searches the Event in m_EventDescription
//## and if included 
bool mitk::EventMapper::MapEvent(Event* event)
{
	if (m_GlobalStateMachine == NULL)
		return false;

	unsigned int i;
	for (i = 0;
		(i < m_EventDescriptions.size()) &&
		!(m_EventDescriptions[i] == *event);//insecure[] but .at(i) is not supported before std.vers 3.0
		i++){}
	if (!(m_EventDescriptions[i] == *event))//searched entry not found
		return false;
	//generate StateEvent and send to StateMachine, which does everything further!
	m_StateEvent.Set( (m_EventDescriptions[i]).GetId(), event );
	return m_GlobalStateMachine->HandleEvent(&m_StateEvent);
}

//##ModelId=3E5B35140072
//##Documentation
//## loads an XML-File containing Events into m_EventDescriptions
//## also involved: EventMapper::startEvent(...)
bool mitk::EventMapper::LoadBehavior(std::string fileName)
{
	  if ( fileName.empty() )
       return false;

   QFile xmlFile( fileName.c_str() );
   if ( !xmlFile.exists() )
       return false;

   QXmlInputSource source( &xmlFile );
   QXmlSimpleReader reader;
   mitk::EventMapper* eventMapper = new EventMapper();
   reader.setContentHandler( eventMapper );
   reader.parse( source );

   delete eventMapper;
   return true;
}

//##ModelId=3E788FC00308
//##Documentation
//## reads a Tag from an XML-file
//## adds Events to m_EventDescription
bool mitk::EventMapper::startElement( const QString&, const QString&, const QString& qName, const QXmlAttributes& atts )
{

	//qName.ascii(); //for debuging
	if( qName == "events")
	{
		m_StyleName = atts.value( STYLE.c_str() ).latin1();
	}
	else if ( qName == "event")
	{
		//neuen Eintrag in der m_EventDescriptions
		bool ok;
		EventDescription tempEventDescr( atts.value( TYPE.c_str() ).toInt(),
										 (( atts.value((QString)(BUTTON.c_str())) ).remove(0,2) ).toInt( &ok, 16 ),
										 (( atts.value((QString)(BUTTONSTATE.c_str())) ).remove(0,2) ).toInt( &ok, 16 ),
										 (( atts.value((QString)(KEY.c_str())) ).remove(0,2) ).toInt( &ok, 16 ),
										 atts.value( NAME.c_str() ).latin1(),
										 atts.value( ID.c_str() ).toInt());
		if (!ok) std::cout<<"error reading Event::Button, ButtonState or Key!"<<std::endl;
		m_EventDescriptions.push_back(tempEventDescr);
	}
	return true;
}

//##ModelId=3E7B20EE01F5
std::string mitk::EventMapper::GetStyleName()
{
	return m_StyleName;
}

/*
16.05.2003 QT-FAQ:http://lists.trolltech.com/qt-interest/2001-03/thread00611-0.html
Trolltech Home |  Qt-interest Home | Recent Threads | All Threads | Author | Date 	
All threads index page 5
Qt-interest Archive, March 2001
How to receive event's name?
Pages: Prev | 1 | 2 | Next
Message 1 in thread

    * Subject: How to receive event's name?
    * From: "Denis Yu. Konovalchik" <D.Konovalchik@xxxxxxxxxxxxxx>
    * Date: Tue, 20 Mar 2001 12:18:50 +0500
    * Organization: Compass Plus
    * Reply-to: =?koi8-r?B?5MXOydMg68/Oz9fBzNjeycs=?= <D.Konovalchik@xxxxxxxxxxxxxx>
    * To: qt-interest@xxxxxxxxxxxxx

Dobry den' ;-)

Not too long ago I've decided to make a special debug window for my
application to catch and translate specific Qt events, and met the
problem.
Is there a way to obtain the Qt event's name by its type() value (for example,
 9 => "FocusOut", 16=> "Destroy") without the big case operator with
 the manual translation of each event type?

-- 
 [ signature omitted ] 

Message 2 in thread

    * Subject: Re: How to receive event's name?
    * From: "Roy Dennington" <roy@xxxxxxxxxxxx>
    * Date: Tue, 20 Mar 2001 08:23:29 -0600
    * Cc: <D.Konovalchik@xxxxxxxxxxxxxx>
    * To: "Qt-Interest" <qt-interest@xxxxxxxxxxxxx>

Denis,

> Is there a way to obtain the Qt event's name by its type() value (for
example,
>  9 => "FocusOut", 16=> "Destroy") without the big case operator with
>  the manual translation of each event type?

Here is a more elegant solution (untested and incomplete). You will have
to load event_array into event_string during initialization.

struct EventArray
{
  int type;
  char* string;
};

EventArray event_array[] =
{
  { QEvent::None, "None" },
  { QEvent::Timer, "Timer" },
  { QEvent::MouseButtonPress, "MouseButtonPress" },
  { QEvent::MouseButtonRelease, "MouseButtonRelease" },
  { QEvent::MouseButtonDblClick, "MouseButtonDblClick" },
  { QEvent::MouseMove, "MouseMove" },
  { QEvent::KeyPress, "KeyPress" },
  { QEvent::KeyRelease, "KeyRelease" },
  { QEvent::FocusIn, "FocusIn" },
  { QEvent::FocusOut, "FocusOut" },
  { QEvent::Enter, "Enter" },
  { QEvent::Leave, "Leave" },
  { QEvent::Paint, "Paint" },
  { QEvent::Move, "Move" },
  { QEvent::Resize, "Resize" },
  { QEvent::Create, "Create" },
  { QEvent::Destroy, "Destroy" },
  { QEvent::Show, "Show" },
  { QEvent::Hide, "Hide" },
  { QEvent::Close, "Close" },
  { QEvent::Quit, "Quit" },
  { QEvent::Reparent, "Reparent" },
  { QEvent::ShowMinimized, "ShowMinimized" },
  { QEvent::ShowNormal, "ShowNormal" },
  { QEvent::WindowActivate, "WindowActivate" },
  { QEvent::WindowDeactivate, "WindowDeactivate" },
  { QEvent::ShowToParent, "ShowToParent" },
  { QEvent::HideToParent, "HideToParent" },
  { QEvent::ShowMaximized, "ShowMaximized" },
  { QEvent::Accel, "Accel" },
  { QEvent::Wheel, "Wheel" },
  { QEvent::AccelAvailable, "AccelAvailable" },
  { QEvent::CaptionChange, "CaptionChange" },
  { QEvent::IconChange, "IconChange" },
  { QEvent::ParentFontChange, "ParentFontChange" },
  { QEvent::ApplicationFontChange, "ApplicationFontChange" },
  { QEvent::ParentPaletteChange, "ParentPaletteChange" },
  { QEvent::ApplicationPaletteChange, "ApplicationPaletteChange" },
  { QEvent::Clipboard, "Clipboard" },
  { QEvent::Speech, "Speech" },
  { QEvent::SockAct, "SockAct" },
  { QEvent::AccelOverride, "AccelOverride" },
  { QEvent::DragEnter, "DragEnter" },
  { QEvent::DragMove, "DragMove" },
  { QEvent::DragLeave, "DragLeave" },
  { QEvent::Drop, "Drop" },
  { QEvent::DragResponse, "DragResponse" },
  { QEvent::ChildInserted, "ChildInserted" },
  { QEvent::ChildRemoved, "ChildRemoved" },
  { QEvent::LayoutHint, "LayoutHint" },
  { QEvent::ShowWindowRequest, "ShowWindowRequest" },
  { QEvent::ActivateControl, "ActivateControl" },
  { QEvent::DeactivateControl, "DeactivateControl" },
  { QEvent::User, "User" },
  { 0, 0 }
};

QIntDict<char> event_string(59);

int n = -1;
while ( event_array[++n].string )
  event_string.insert( event_array[n].type, event_array[n].string );

To use:
  char* str = event_string[event];  // very efficient!

Enjoy,
Roy Dennington
roy@semichem.com



Message 3 in thread

    * Subject: Re: How to receive event's name?
    * From: Johann Gunnar Oskarsson <johanngo@xxxxxxxxxxx>
    * Date: Tue, 20 Mar 2001 15:49:01 +0000
    * Cc: <D.Konovalchik@xxxxxxxxxxxxxx>, "Roy Dennington" <roy@xxxxxxxxxxxx>
    * Organization: Firmanet
    * To: "Qt-Interest" <qt-interest@xxxxxxxxxxxxx>

Hi,

On Tuesday 20 March 2001 14:23, Roy Dennington wrote:
> Denis,
>
> > Is there a way to obtain the Qt event's name by its type() value (for
>
> example,
>
> >  9 => "FocusOut", 16=> "Destroy") without the big case operator with
> >  the manual translation of each event type?
>
> Here is a more elegant solution (untested and incomplete). You will have
> to load event_array into event_string during initialization.

This is, IMO, a lot better and safer:

// Declaration:
std::map< int, std::string > event_map;

// Run time initialization (has to be in a function or constructor):

event_map[QEvent::None] = "None";
event_map[QEvent::Timer] = "Timer";
event_map[QEvent::MouseButtonPress] = "MouseButtonPress";
event_map[QEvent::MouseButtonRelease] = "MouseButtonRelease";
event_map[QEvent::MouseButtonDblClick] = "MouseButtonDblClick";
event_map[QEvent::MouseMove] = "MouseMove";
event_map[QEvent::KeyPress] = "KeyPress";
event_map[QEvent::KeyRelease] = "KeyRelease";
event_map[QEvent::FocusIn] = "FocusIn";
event_map[QEvent::FocusOut] = "FocusOut";
event_map[QEvent::Enter] = "Enter";
event_map[QEvent::Leave] = "Leave";
event_map[QEvent::Paint] = "Paint";
event_map[QEvent::Move] = "Move";
event_map[QEvent::Resize] = "Resize";
event_map[QEvent::Create] = "Create";
event_map[QEvent::Destroy] = "Destroy";
event_map[QEvent::Show] = "Show";
event_map[QEvent::Hide] = "Hide";
event_map[QEvent::Close] = "Close";
event_map[QEvent::Quit] = "Quit";
event_map[QEvent::Reparent] = "Reparent";
event_map[QEvent::ShowMinimized] = "ShowMinimized";
event_map[QEvent::ShowNormal] = "ShowNormal";
event_map[QEvent::WindowActivate] = "WindowActivate";
event_map[QEvent::WindowDeactivate] = "WindowDeactivate";
event_map[QEvent::ShowToParent] = "ShowToParent";
event_map[QEvent::HideToParent] = "HideToParent";
event_map[QEvent::ShowMaximized] = "ShowMaximized";
event_map[QEvent::Accel] = "Accel";
event_map[QEvent::Wheel] = "Wheel";
event_map[QEvent::AccelAvailable] = "AccelAvailable";
event_map[QEvent::CaptionChange] = "CaptionChange";
event_map[QEvent::IconChange] = "IconChange";
event_map[QEvent::ParentFontChange] = "ParentFontChange";
event_map[QEvent::ApplicationFontChange] = "ApplicationFontChange";
event_map[QEvent::ParentPaletteChange] = "ParentPaletteChange";
event_map[QEvent::ApplicationPaletteChange] = "ApplicationPaletteChange";
event_map[QEvent::Clipboard] = "Clipboard";
event_map[QEvent::Speech] = "Speech";
event_map[QEvent::SockAct] = "SockAct";
event_map[QEvent::AccelOverride] = "AccelOverride";
event_map[QEvent::DragEnter] = "DragEnter";
event_map[QEvent::DragMove] = "DragMove";
event_map[QEvent::DragLeave] = "DragLeave";
event_map[QEvent::Drop] = "Drop";
event_map[QEvent::DragResponse] = "DragResponse";
event_map[QEvent::ChildInserted] = "ChildInserted";
event_map[QEvent::ChildRemoved] = "ChildRemoved";
event_map[QEvent::LayoutHint] = "LayoutHint";
event_map[QEvent::ShowWindowRequest] = "ShowWindowRequest";
event_map[QEvent::ActivateControl] = "ActivateControl";
event_map[QEvent::DeactivateControl] = "DeactivateControl";
event_map[QEvent::User] = "User";
event_map[0] = 0;


//Usage
std::string = event_map[QEvent::User];  // very efficient

Johann

// Roy Dennington's solution:
>
> struct EventArray
> {
>   int type;
>   char* string;
> };
>
> EventArray event_array[] =
> {
>   { QEvent::None, "None" },
>   { QEvent::Timer, "Timer" },

[snip]
You get the idea

>   { QEvent::ActivateControl, "ActivateControl" },
>   { QEvent::DeactivateControl, "DeactivateControl" },
>   { QEvent::User, "User" },
>   { 0, 0 }
> };
>
> QIntDict<char> event_string(59);
>
> int n = -1;
> while ( event_array[++n].string )
>   event_string.insert( event_array[n].type, event_array[n].string );
>
> To use:
>   char* str = event_string[event];  // very efficient!
>
> Enjoy,
> Roy Dennington
> roy@semichem.com
>

Message 4 in thread

    * Subject: Re: How to receive event's name?
    * From: Rik Hemsley <rik@xxxxxxx>
    * Date: Tue, 20 Mar 2001 18:15:13 +0000
    * Mail-followup-to: Qt-Interest <qt-interest@trolltech.com>
    * To: Qt-Interest <qt-interest@xxxxxxxxxxxxx>

#if Johann Gunnar Oskarsson

> This is, IMO, a lot better and safer:
> 
> // Declaration:
> std::map< int, std::string > event_map;

And using QMap saves you linking STL :)

Rik


Message 5 in thread

    * Subject: Re: How to receive event's name?
    * From: Guillaume Laurent <glaurent@xxxxxxxxxxxxxxxxxx>
    * Date: Tue, 20 Mar 2001 23:12:11 +0100
    * To: qt-interest@xxxxxxxxxxxxx

On Tuesday 20 March 2001 19:15, Rik Hemsley wrote:

> And using QMap saves you linking STL :)

The STL is not a library you link with :-).

In any case, I doubt the difference between QMap and std::map (or hash_map 
since it provided most of the time) is really significant (probably depends 
on which implementation of the STL you're using, though).

-- 
 [ signature omitted ] 

Message 6 in thread

    * Subject: QListView::clear() not working
    * From: "Neil Barman" <neil@xxxxxxx>
    * Date: Tue, 20 Mar 2001 18:54:00 -0600
    * To: <qt-interest@xxxxxxxxxxxxx>

Hi,

"clear" is not working for QListView. I will appreciate if you please let me
know what I am doing wrong.


QListView *lvSched;


{

   lvSched->clear();


add stuff using QListViewItem ..
   QListViewItem *lvi = new QListViewItem (lvSched, "something",...)


}

The program is not clearing the ListView when I come to the "clear" again.
It is adding on top of the previous one.


Any help will be appreciated.

Regards
Nil


Message 7 in thread

    * Subject: Re: QListView::clear() not working
    * From: wim delvaux <wim.delvaux@xxxxxxxxx>
    * Date: Wed, 21 Mar 2001 02:35:45 +0100
    * Cc: qt-interest@xxxxxxxxxxxxx
    * Organization: Adaptive Planet
    * Sender: u19809@xxxxxxxxxxx
    * To: neil@xxxxxxx


<<< text/html: EXCLUDED >>>
Message 8 in thread

    * Subject: Re: QListView::clear() not working
    * From: "Neil Barman" <neil@xxxxxxx>
    * Date: Wed, 21 Mar 2001 10:07:04 -0600
    * Cc: <qt-interest@xxxxxxxxxxxxx>
    * To: <wim.delvaux@xxxxxxxxx>, <neil@xxxxxxx>


<<< text/html: EXCLUDED >>>
Message 9 in thread

    * Subject: Re: How to receive event's name?
    * From: Rik Hemsley <rik@xxxxxxx>
    * Date: Wed, 21 Mar 2001 01:49:54 +0000
    * Mail-followup-to: qt-interest@trolltech.com
    * To: qt-interest@xxxxxxxxxxxxx

#if Guillaume Laurent
> On Tuesday 20 March 2001 19:15, Rik Hemsley wrote:
> 
> > And using QMap saves you linking STL :)
> 
> The STL is not a library you link with :-).

Well, what I mean is that it saves you having to use STL at all, which
is a good thing because STL isn't as standard as it's cracked up to be.
I prefer to use Qt classes everywhere in my Qt apps, because then
they're guaranteed to be portable to all the places Qt works.

Rik


Message 10 in thread

    * Subject: Re: How to receive event's name?
    * From: Guillaume Laurent <glaurent@xxxxxxxxxxxxxxxxxx>
    * Date: Wed, 21 Mar 2001 13:16:00 +0100
    * To: qt-interest@xxxxxxxxxxxxx

On Wednesday 21 March 2001 02:49, Rik Hemsley wrote:

> Well, what I mean is that it saves you having to use STL at all, which
> is a good thing because STL isn't as standard as it's cracked up to be.

It is now. Every newly released C++ compiler supports it AFAIK. Though I 
think even gcc 2.7 supports map<> adequately.

> I prefer to use Qt classes everywhere in my Qt apps, because then
> they're guaranteed to be portable to all the places Qt works.

Agreed. This is the recurring debate on whether one should support broken 
compilers or not. There is no definitive answer on that.

-- 
 [ signature omitted ] 

Message 11 in thread

    * Subject: Re: How to receive event's name?
    * From: "Roy Dennington" <roy@xxxxxxxxxxxx>
    * Date: Wed, 21 Mar 2001 09:13:03 -0600
    * To: "Qt-Interest" <qt-interest@xxxxxxxxxxxxx>

All,

> On Wednesday 21 March 2001 02:49, Rik Hemsley wrote:
>
> > Well, what I mean is that it saves you having to use STL at all, which
> > is a good thing because STL isn't as standard as it's cracked up to be.
>
> It is now. Every newly released C++ compiler supports it AFAIK. Though I
> think even gcc 2.7 supports map<> adequately.

Yes, but your mileage for STL varies from one compiler to the next. Recall,
even STL evolved from "draft" to "final". Compilers are catching up, but
it is risky to say STL is standard and stable everywhere. STL comes with
alot of baggage:    STL=>Exception Handling => std namespace.

I can tell you that a recent SGI compiler does not adequately handle
std namespace. Nor does it like "#include <iostream>". It seems to
support STL somewhere in between draft and final.

The HP compiler (aCC) is also somewhat behind.

These are major platforms that my company cannot ignore.

> Agreed. This is the recurring debate on whether one should support broken
> compilers or not. There is no definitive answer on that.

We have begun to include small pieces of STL code to "test the water".
But we cannot afford to exclude a platform simply because the compilers
are behind the standard. Once you adopt STL, you cannot easily UNDO.

I look forward to using STL eventually.

Regards,
Roy Dennington
roy@semichem.com



Message 12 in thread

    * Subject: Re: How to receive event's name?
    * From: Matthias Stiller <stiller@xxxxxxxx>
    * Date: Wed, 21 Mar 2001 16:19:05 +0100 (MET)
    * Reply-to: stiller@xxxxxxxx
    * To: roy@xxxxxxxxxxxx, "Qt-Interest" <qt-interest@xxxxxxxxxxxxx>

On Mar 21,  9:13am, Roy Dennington wrote:
> Subject: Re: How to receive event's name?
> All,
>
> > On Wednesday 21 March 2001 02:49, Rik Hemsley wrote:
> >
> > > Well, what I mean is that it saves you having to use STL at all, which
> > > is a good thing because STL isn't as standard as it's cracked up to be.
> >
> > It is now. Every newly released C++ compiler supports it AFAIK. Though I
> > think even gcc 2.7 supports map<> adequately.
>
> Yes, but your mileage for STL varies from one compiler to the next. Recall,
> even STL evolved from "draft" to "final". Compilers are catching up, but
> it is risky to say STL is standard and stable everywhere. STL comes with
> alot of baggage:    STL=>Exception Handling => std namespace.
>
> I can tell you that a recent SGI compiler does not adequately handle
> std namespace. Nor does it like "#include <iostream>". It seems to
> support STL somewhere in between draft and final.
>
> The HP compiler (aCC) is also somewhat behind.

Hello,

just a short question. Why do you stick to the STL delivered together with your
compiler ? The STL are just header files, so you could download a new version
from oss.sgi.com anytime.

The problem with #include <iostream> can be resolved if you just use the
following compiler switch -LANG:std

Regards

Matthias
--
 [ signature omitted ] 

Message 13 in thread

    * Subject: Re: How to receive event's name?
    * From: "Philippe A. Bouchard" <jh82vyof@xxxxxxxxxxxxxxx>
    * Date: Wed, 21 Mar 2001 07:47:41 -0500
    * Organization: Corel Linux
    * To: <qt-interest@xxxxxxxx>

Rik Hemsley wrote:

> #if Guillaume Laurent
> > On Tuesday 20 March 2001 19:15, Rik Hemsley wrote:
> >
> > > And using QMap saves you linking STL :)
> >
> > The STL is not a library you link with :-).
>
> Well, what I mean is that it saves you having to use STL at all, which
> is a good thing because STL isn't as standard as it's cracked up to be.
> I prefer to use Qt classes everywhere in my Qt apps, because then
> they're guaranteed to be portable to all the places Qt works.
>
> Rik
>
> --
> List archive and information: http://qt-interest.trolltech.com

It's true, but STL is so much powerfull. You have complex algorithms
already implemented, the abstraction is so much beautifull that once you
get in touch with it you don't want to let it go. One thing I find sad
from QT is that QT streams are not compatible at all with standard C++
streams (if there is a compiler out there that is not supporting C++
streams, my bets on this compiler would be lower than ever).

If QT was to strenghten their abstraction layer, they would have a lot
more chances of becoming a standard for UNIX platforms than GTK.

--
 [ signature omitted ] 

Message 14 in thread

    * Subject: Re: How to receive event's name?
    * From: Lucio Ismael Flores <ismael@xxxxxx>
    * Date: Wed, 21 Mar 2001 10:03:17 -0800
    * Cc: qt-interest@xxxxxxxxxxxxx
    * Organization: Digital Domain
    * Reply-to: ismael@xxxxxx
    * Sender: ismael@xxxxxx
    * To: rik@xxxxxxx

Rik Hemsley wrote:

> #if Guillaume Laurent
> > On Tuesday 20 March 2001 19:15, Rik Hemsley wrote:
> >
> > > And using QMap saves you linking STL :)
> >
> > The STL is not a library you link with :-).
>
> Well, what I mean is that it saves you having to use STL at all, which
> is a good thing because STL isn't as standard as it's cracked up to be.
> I prefer to use Qt classes everywhere in my Qt apps, because then
> they're guaranteed to be portable to all the places Qt works.
>

Hmmm... STL IS a C++ standard. What isn't standard is the signal/slot

mechanism, which is Troll-Tech's own extension to C++.

--
 [ signature omitted ] 

Message 15 in thread

    * Subject: Re: How to receive event's name?
    * From: "Aigars Grins" <aigars.grins@xxxxxxxxxx>
    * Date: Wed, 21 Mar 2001 18:53:29 -0000
    * Organization: Defcom
    * To: <qt-interest@xxxxxxxxxxxxx>

> > I prefer to use Qt classes everywhere in my Qt apps, because then
> > they're guaranteed to be portable to all the places Qt works.
>
> Hmmm... STL IS a C++ standard. What isn't standard is the signal/slot
> mechanism, which is Troll-Tech's own extension to C++.

I like the STL and it _is_ part of the 'standard'. The problem is that
_very_ few compilers are _100_% compatible to the standard. Most cover
something like 90-98%. The problem is of course getting smaller and smaller
over time. Nowadays the STL works ok for most compilers/platforms (but not
everything everywhere).

Using Troll specific containers etc. gives you a guarantee that they work
everywhere where Qt works. That is a Good Thing.

Having a way of giving members functions as callbacks is almost always very
appreciated in most GUI libs. This isn't supported by the language in any
specific way (not in a simple and efficient way) and since things like
libsigc isn't that well developed for xplatform functionality (at least yet)
most libs/IDEs will have to come up with something of their own. Trolls way
is signals/slots.

All this is obvious and old news. But it brings me back again to the wish
that Troll could separate their non-GUI code from the rest. I've been told
(by Jasmin) why they wont. But still. It would be great. (Even better would
be somthing like a good xplatform ATL).

--
 [ signature omitted ] 

Pages: Prev | 1 | 2 | Next


*/