/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


/**
*   EventMapping:
*       This class mapps the Events, usually given by the OS or here by QT, to a MITK internal EventId.
*       It loads all information from the xml-file (possible, understandable Events with the mitkEventID).
*       If an event appears, the method MapEvent is called with the event params.
*       This Method looks up the event params, and tries to find an mitkEventId to it.
*       If yes, then sends the event and the found ID to the globalStateMachine, which handles all
*       further operations of that event.
*
*/

#include "mitkEventMapper.h"
#include <itkOutputWindow.h>
#include <itkMacro.h>
//#include "mitkStatusBar.h"
#include "mitkInteractionConst.h"
#include "mitkStateEvent.h"
#include "mitkOperationEvent.h"
#include "mitkStateMachine.h"

#include "mitkGlobalInteraction.h"
#include <mitkStandardFileLocations.h>

//#include <mitkInteractionDebugger.h>
#include <mitkConfig.h>
#include <itksys/SystemTools.hxx>
#include "mitkCoreObjectFactory.h"

#include <vtkObjectFactory.h>
namespace mitk
{
vtkStandardNewMacro(EventMapper);
}

#ifdef MBI_INTERNAL_CONFERENCE
  #include <mitkGeometry3D.h>
  #include <mitkDisplayPositionEvent.h>

  #include <mitkConferenceKit.h>
  #include <mitkConferenceToken.h>
  #include <mitkBaseRenderer.h>
#endif //MBI_INTERNAL_CONFERENCE

//XML Event
const std::string mitk::EventMapper::STYLE = "STYLE";
const std::string mitk::EventMapper::NAME = "NAME";
const std::string mitk::EventMapper::ID = "ID";
const std::string mitk::EventMapper::TYPE = "TYPE";
const std::string mitk::EventMapper::BUTTON = "BUTTON";
const std::string mitk::EventMapper::BUTTONSTATE = "BUTTONSTATE";
const std::string mitk::EventMapper::KEY = "KEY";

const std::string mitk::EventMapper::EVENTS = "events";

const std::string mitk::EventMapper::EVENT = "event";

mitk::StateMachine *mitk::EventMapper::s_GlobalStateMachine(NULL);

mitk::EventMapper::EventDescriptionVec mitk::EventMapper::m_EventDescriptions;

mitk::StateEvent mitk::EventMapper::m_StateEvent;

std::string mitk::EventMapper::m_StyleName;



struct ltstr
{
  bool operator()(const char* s1, const char* s2) const
  {
    return strcmp(s1, s2) < 0;
  }
};

mitk::EventMapper::EventMapper()
{

  //map with string to key for mapping string from xml-file to int
  m_EventConstMap["Type_None"] = mitk::Type_None;               // invalid event
  m_EventConstMap["Type_Timer"] = mitk::Type_Timer;             // timer event
  m_EventConstMap["Type_MouseButtonPress"] = mitk::Type_MouseButtonPress;               // mouse button pressed
  m_EventConstMap["Type_MouseButtonRelease"] = mitk::Type_MouseButtonRelease;           // mouse button released
  m_EventConstMap["Type_MouseButtonDblClick"] = mitk::Type_MouseButtonDblClick;         // mouse button double click
  m_EventConstMap["Type_MouseMove"] = mitk::Type_MouseMove;             // mouse move
  m_EventConstMap["Type_KeyPress"] = mitk::Type_KeyPress;               // key pressed
  m_EventConstMap["Type_KeyRelease"] = mitk::Type_KeyRelease;           // key released
  m_EventConstMap["Type_FocusIn"] =                             8;              // keyboard focus received
  m_EventConstMap["Type_FocusOut"] =                            9;              // keyboard focus lost
  m_EventConstMap["Type_Enter"] =                                       10;     // mouse enters widget
  m_EventConstMap["Type_Leave"] =                                       11;     // mouse leaves widget
  m_EventConstMap["Type_Paint"] =                                       12;     // paint widget
  m_EventConstMap["Type_Move"] =                                        13;     // move widget
  m_EventConstMap["Type_Resize"] =                              14;     // resize widget
  m_EventConstMap["Type_Create"] =                              15;     // after object creation
  m_EventConstMap["Type_Destroy"] =                             16;     // during object destruction
  m_EventConstMap["Type_Show"] =                                        17;     // widget is shown
  m_EventConstMap["Type_Hide"] =                                        18;     // widget is hidden
  m_EventConstMap["Type_Close"] =                                       19;     // request to close widget
  m_EventConstMap["Type_Quit"] =                                        20;     // request to quit application
  m_EventConstMap["Type_Reparent"] =                            21;     // widget has been reparented
  m_EventConstMap["Type_ShowMinimized"] =                       22;     // widget is shown minimized
  m_EventConstMap["Type_ShowNormal"] =                  23;     // widget is shown normal
  m_EventConstMap["Type_WindowActivate"] =              24;     // window was activated
  m_EventConstMap["Type_WindowDeactivate"] =            25;     // window was deactivated
  m_EventConstMap["Type_ShowToParent"] =                        26;     // widget is shown to parent
  m_EventConstMap["Type_HideToParent"] =                        27;     // widget is hidden to parent
  m_EventConstMap["Type_ShowMaximized"] =                       28;     // widget is shown maximized
  m_EventConstMap["Type_ShowFullScreen"] =              29;     // widget is shown full-screen
  m_EventConstMap["Type_Accel"] =                                       30;     // accelerator event
  m_EventConstMap["Type_Wheel"] =                                       31;     // wheel event
  m_EventConstMap["Type_AccelAvailable"] =              32;     // accelerator available event
  m_EventConstMap["Type_CaptionChange"] =                       33;     // caption changed
  m_EventConstMap["Type_IconChange"] =                  34;     // icon changed
  m_EventConstMap["Type_ParentFontChange"] =            35;     // parent font changed
  m_EventConstMap["Type_ApplicationFontChange"] =       36;// application font changed
  m_EventConstMap["Type_ParentPaletteChange"] =         37;     // parent palette changed
  m_EventConstMap["Type_ApplicationPaletteChange"] =  38;// application palette changed
  m_EventConstMap["Type_PaletteChange"] =                       39;     // widget palette changed
  m_EventConstMap["Type_Clipboard"] =                           40;     // internal clipboard event
  m_EventConstMap["Type_Speech"] =                              42;     // reserved for speech input
  m_EventConstMap["Type_SockAct"] =                             50;     // socket activation
  m_EventConstMap["Type_AccelOverride"] =                       51;     // accelerator override event
  m_EventConstMap["Type_DeferredDelete"] =              52;     // deferred delete event
  m_EventConstMap["Type_DragEnter"] =                           60;     // drag moves into widget
  m_EventConstMap["Type_DragMove"] =                            61;     // drag moves in widget
  m_EventConstMap["Type_DragLeave"] =                           62;     // drag leaves or is cancelled
  m_EventConstMap["Type_Drop"] =                                        63;     // actual drop
  m_EventConstMap["Type_DragResponse"] =                        64;     // drag accepted/rejected
  m_EventConstMap["Type_ChildInserted"] =                       70;     // new child widget
  m_EventConstMap["Type_ChildRemoved"] =                        71;     // deleted child widget
  m_EventConstMap["Type_LayoutHint"] =                  72;     // child min/max size changed
  m_EventConstMap["Type_ShowWindowRequest"] =           73;     // widget's window should be mapped
  m_EventConstMap["Type_ActivateControl"] =             80;     // ActiveX activation
  m_EventConstMap["Type_DeactivateControl"] =           81;     // ActiveX deactivation
  m_EventConstMap["Type_ContextMenu"] =                         82;     // context popup menu
  m_EventConstMap["Type_IMStart"] =                             83;     // input method composition start
  m_EventConstMap["Type_IMCompose"] =                           84;     // input method composition
  m_EventConstMap["Type_IMEnd"] =                                       85;     // input method composition end
  m_EventConstMap["Type_Accessibility"] =                       86;     // accessibility information is requested
  m_EventConstMap["Type_TabletMove"] =                  87;     // Wacom tablet event
  m_EventConstMap["Type_LocaleChange"] =                        88;     // the system locale changed
  m_EventConstMap["Type_LanguageChange"] =              89;     // the application language changed
  m_EventConstMap["Type_LayoutDirectionChange"] =       90;     // the layout direction changed
  m_EventConstMap["Type_Style"] =                                       91;     // internal style event
  m_EventConstMap["Type_TabletPress"] =                         92;     // tablet press
  m_EventConstMap["Type_TabletRelease"] =                       93;     // tablet release
  m_EventConstMap["Type_TDMouseInput"] =                        mitk::Type_TDMouseInput;     // 3D mouse input occured
  m_EventConstMap["Type_User"] =                                        1000;   // first user event id
  m_EventConstMap["Type_MaxUser"] =                             65535;  // last user event id

  //ButtonState
  m_EventConstMap["BS_NoButton"] =  mitk::BS_NoButton;//0x0000
  m_EventConstMap["BS_LeftButton"] =  mitk::BS_LeftButton;//0x0001
  m_EventConstMap["BS_RightButton"] =  mitk::BS_RightButton;//0x0002
  m_EventConstMap["BS_MidButton"] =  mitk::BS_MidButton;//0x0004
  m_EventConstMap["BS_MouseButtonMask"] =  mitk::BS_MouseButtonMask;//0x0007
  m_EventConstMap["BS_ShiftButton"] =  mitk::BS_ShiftButton;//0x0008
  m_EventConstMap["BS_ControlButton"] =  mitk::BS_ControlButton;//0x0010
  m_EventConstMap["BS_AltButton"] =  mitk::BS_AltButton;//0x0020
  m_EventConstMap["BS_KeyButtonMask"] =  mitk::BS_KeyButtonMask;//0x0038
  m_EventConstMap["BS_Keypad"] =  mitk::BS_Keypad;//0x4000

  //Modifier
  m_EventConstMap["Mod_SHIFT"] =  0x00200000;
  m_EventConstMap["Mod_CTRL"] =  0x00400000;
  m_EventConstMap["Mod_ALT"] =  0x00800000;
  m_EventConstMap["Mod_MODIFIER_MASK"] =  0x00e00000;
  m_EventConstMap["Mod_UNICODE_ACCEL"] =  0x10000000;
  m_EventConstMap["Mod_ASCII_ACCEL"] =  0x10000000;

  //Key
  m_EventConstMap["Key_Escape"] = 0x1000;
  m_EventConstMap["Key_Tab"] = 0x1001;
  m_EventConstMap["Key_Backtab"] = 0x1002;
  m_EventConstMap["Key_BackTab"] = 0x1002;
  m_EventConstMap["Key_Backspace"] = 0x1003;
  m_EventConstMap["Key_BackSpace"] = 0x1003;
  m_EventConstMap["Key_Return"] = 0x1004;
  m_EventConstMap["Key_Enter"] = 0x1005;
  m_EventConstMap["Key_Insert"] = 0x1006;
  m_EventConstMap["Key_Delete"] = 0x1007;
  m_EventConstMap["Key_Pause"] = 0x1008;
  m_EventConstMap["Key_Print"] = 0x1009;
  m_EventConstMap["Key_SysReq"] = 0x100a;
  m_EventConstMap["Key_Home"] = 0x1010;
  m_EventConstMap["Key_End"] = 0x1011;
  m_EventConstMap["Key_Left"] = 0x1012;
  m_EventConstMap["Key_Up"] = 0x1013;
  m_EventConstMap["Key_Right"] = 0x1014;
  m_EventConstMap["Key_Down"] = 0x1015;
  m_EventConstMap["Key_Prior"] = 0x1016;
  m_EventConstMap["Key_PageUp"] = 0x1016;
  m_EventConstMap["Key_Next"] = 0x1017;
  m_EventConstMap["Key_PageDown"] = 0x1017;
  m_EventConstMap["Key_Shift"] = 0x1020;
  m_EventConstMap["Key_Control"] = 0x1021;
  m_EventConstMap["Key_Meta"] = 0x1022;
  m_EventConstMap["Key_Alt"] = 0x1023;
  m_EventConstMap["Key_CapsLock"] = 0x1024;
  m_EventConstMap["Key_NumLock"] = 0x1025;
  m_EventConstMap["Key_ScrollLock"] = 0x1026;
  m_EventConstMap["Key_F1"] = 0x1030;
  m_EventConstMap["Key_F2"] = 0x1031;
  m_EventConstMap["Key_F3"] = 0x1032;
  m_EventConstMap["Key_F4"] = 0x1033;
  m_EventConstMap["Key_F5"] = 0x1034;
  m_EventConstMap["Key_F6"] = 0x1035;
  m_EventConstMap["Key_F7"] = 0x1036;
  m_EventConstMap["Key_F8"] = 0x1037;
  m_EventConstMap["Key_F9"] = 0x1038;
  m_EventConstMap["Key_F10"] = 0x1039;
  m_EventConstMap["Key_F11"] = 0x103a;
  m_EventConstMap["Key_F12"] = 0x103b;
  m_EventConstMap["Key_F13"] = 0x103c;
  m_EventConstMap["Key_F14"] = 0x103d;
  m_EventConstMap["Key_F15"] = 0x103e;
  m_EventConstMap["Key_F16"] = 0x103f;
  m_EventConstMap["Key_F17"] = 0x1040;
  m_EventConstMap["Key_F18"] = 0x1041;
  m_EventConstMap["Key_F19"] = 0x1042;
  m_EventConstMap["Key_F20"] = 0x1043;
  m_EventConstMap["Key_F21"] = 0x1044;
  m_EventConstMap["Key_F22"] = 0x1045;
  m_EventConstMap["Key_F23"] = 0x1046;
  m_EventConstMap["Key_F24"] = 0x1047;
  m_EventConstMap["Key_F25"] = 0x1048;
  m_EventConstMap["Key_F26"] = 0x1049;
  m_EventConstMap["Key_F27"] = 0x104a;
  m_EventConstMap["Key_F28"] = 0x104b;
  m_EventConstMap["Key_F29"] = 0x104c;
  m_EventConstMap["Key_F30"] = 0x104d;
  m_EventConstMap["Key_F31"] = 0x104e;
  m_EventConstMap["Key_F32"] = 0x104f;
  m_EventConstMap["Key_F33"] = 0x1050;
  m_EventConstMap["Key_F34"] = 0x1051;
  m_EventConstMap["Key_F35"] = 0x1052;
  m_EventConstMap["Key_Super_L"] = 0x1053;
  m_EventConstMap["Key_Super_R"] = 0x1054;
  m_EventConstMap["Key_Menu"] = 0x1055;
  m_EventConstMap["Key_Hyper_L"] = 0x1056;
  m_EventConstMap["Key_Hyper_R"] = 0x1057;
  m_EventConstMap["Key_Help"] = 0x1058;
  m_EventConstMap["Key_Muhenkan"] =  0x1122;
  m_EventConstMap["Key_Henkan"] =  0x1123;
  m_EventConstMap["Key_Hiragana_Katakana"] = 0x1127;
  m_EventConstMap["Key_Zenkaku_Hankaku"] = 0x112A;
  m_EventConstMap["Key_Space"] = 0x20;
  m_EventConstMap["Key_Any"] = 0x20;
  m_EventConstMap["Key_Exclam"] = 0x21;
  m_EventConstMap["Key_QuoteDbl"] = 0x22;
  m_EventConstMap["Key_NumberSign"] = 0x23;
  m_EventConstMap["Key_Dollar"] = 0x24;
  m_EventConstMap["Key_Percent"] = 0x25;
  m_EventConstMap["Key_Ampersand"] = 0x26;
  m_EventConstMap["Key_Apostrophe"] = 0x27;
  m_EventConstMap["Key_ParenLeft"] = 0x28;
  m_EventConstMap["Key_ParenRight"] = 0x29;
  m_EventConstMap["Key_Asterisk"] = 0x2a;
  m_EventConstMap["Key_Plus"] = 0x2b;
  m_EventConstMap["Key_Comma"] = 0x2c;
  m_EventConstMap["Key_Minus"] = 0x2d;
  m_EventConstMap["Key_Period"] = 0x2e;
  m_EventConstMap["Key_Slash"] = 0x2f;
  m_EventConstMap["Key_0"] = 0x30;
  m_EventConstMap["Key_1"] = 0x31;
  m_EventConstMap["Key_2"] = 0x32;
  m_EventConstMap["Key_3"] = 0x33;
  m_EventConstMap["Key_4"] = 0x34;
  m_EventConstMap["Key_5"] = 0x35;
  m_EventConstMap["Key_6"] = 0x36;
  m_EventConstMap["Key_7"] = 0x37;
  m_EventConstMap["Key_8"] = 0x38;
  m_EventConstMap["Key_9"] = 0x39;
  m_EventConstMap["Key_Colon"] = 0x3a;
  m_EventConstMap["Key_Semicolon"] = 0x3b;
  m_EventConstMap["Key_Less"] = 0x3c;
  m_EventConstMap["Key_Equal"] = 0x3d;
  m_EventConstMap["Key_Greater"] = 0x3e;
  m_EventConstMap["Key_Question"] = 0x3f;
  m_EventConstMap["Key_At"] = 0x40;
  m_EventConstMap["Key_A"] = 0x41;
  m_EventConstMap["Key_B"] = 0x42;
  m_EventConstMap["Key_C"] = 0x43;
  m_EventConstMap["Key_D"] = 0x44;
  m_EventConstMap["Key_E"] = 0x45;
  m_EventConstMap["Key_F"] = 0x46;
  m_EventConstMap["Key_G"] = 0x47;
  m_EventConstMap["Key_H"] = 0x48;
  m_EventConstMap["Key_I"] = 0x49;
  m_EventConstMap["Key_J"] = 0x4a;
  m_EventConstMap["Key_K"] = 0x4b;
  m_EventConstMap["Key_L"] = 0x4c;
  m_EventConstMap["Key_M"] = 0x4d;
  m_EventConstMap["Key_N"] = 0x4e;
  m_EventConstMap["Key_O"] = 0x4f;
  m_EventConstMap["Key_P"] = 0x50;
  m_EventConstMap["Key_Q"] = 0x51;
  m_EventConstMap["Key_R"] = 0x52;
  m_EventConstMap["Key_S"] = 0x53;
  m_EventConstMap["Key_T"] = 0x54;
  m_EventConstMap["Key_U"] = 0x55;
  m_EventConstMap["Key_V"] = 0x56;
  m_EventConstMap["Key_W"] = 0x57;
  m_EventConstMap["Key_X"] = 0x58;
  m_EventConstMap["Key_Y"] = 0x59;
  m_EventConstMap["Key_Z"] = 0x5a;
  m_EventConstMap["Key_BracketLeft"] = 0x5b;
  m_EventConstMap["Key_Backslash"] = 0x5c;
  m_EventConstMap["Key_BracketRight"] = 0x5d;
  m_EventConstMap["Key_AsciiCircum"] = 0x5e;
  m_EventConstMap["Key_Underscore"] = 0x5f;
  m_EventConstMap["Key_QuoteLeft"] = 0x60;
  m_EventConstMap["Key_BraceLeft"] = 0x7b;
  m_EventConstMap["Key_Bar"] = 0x7c;
  m_EventConstMap["Key_BraceRight"] = 0x7d;
  m_EventConstMap["Key_AsciiTilde"] = 0x7e;
  m_EventConstMap["Key_nobreakspace"] = 0x0a0;
  m_EventConstMap["Key_exclamdown"] = 0x0a1;
  m_EventConstMap["Key_cent"] = 0x0a2;
  m_EventConstMap["Key_sterling"] = 0x0a3;
  m_EventConstMap["Key_currency"] = 0x0a4;
  m_EventConstMap["Key_yen"] = 0x0a5;
  m_EventConstMap["Key_brokenbar"] = 0x0a6;
  m_EventConstMap["Key_section"] = 0x0a7;
  m_EventConstMap["Key_diaeresis"] = 0x0a8;
  m_EventConstMap["Key_copyright"] = 0x0a9;
  m_EventConstMap["Key_ordfeminine"] = 0x0aa;
  m_EventConstMap["Key_guillemotleft"] = 0x0ab;
  m_EventConstMap["Key_notsign"] = 0x0ac;
  m_EventConstMap["Key_hyphen"] = 0x0ad;
  m_EventConstMap["Key_registered"] = 0x0ae;
  m_EventConstMap["Key_macron"] = 0x0af;
  m_EventConstMap["Key_degree"] = 0x0b0;
  m_EventConstMap["Key_plusminus"] = 0x0b1;
  m_EventConstMap["Key_twosuperior"] = 0x0b2;
  m_EventConstMap["Key_threesuperior"] = 0x0b3;
  m_EventConstMap["Key_acute"] = 0x0b4;
  m_EventConstMap["Key_mu"] = 0x0b5;
  m_EventConstMap["Key_paragraph"] = 0x0b6;
  m_EventConstMap["Key_periodcentered"] = 0x0b7;
  m_EventConstMap["Key_cedilla"] = 0x0b8;
  m_EventConstMap["Key_onesuperior"] = 0x0b9;
  m_EventConstMap["Key_masculine"] = 0x0ba;
  m_EventConstMap["Key_guillemotright"] = 0x0bb;
  m_EventConstMap["Key_onequarter"] = 0x0bc;
  m_EventConstMap["Key_onehalf"] = 0x0bd;
  m_EventConstMap["Key_threequarters"] = 0x0be;
  m_EventConstMap["Key_questiondown"] = 0x0bf;
  m_EventConstMap["Key_Agrave"] = 0x0c0;
  m_EventConstMap["Key_Aacute"] = 0x0c1;
  m_EventConstMap["Key_Acircumflex"] = 0x0c2;
  m_EventConstMap["Key_Atilde"] = 0x0c3;
  m_EventConstMap["Key_Adiaeresis"] = 0x0c4;
  m_EventConstMap["Key_Aring"] = 0x0c5;
  m_EventConstMap["Key_AE"] = 0x0c6;
  m_EventConstMap["Key_Ccedilla"] = 0x0c7;
  m_EventConstMap["Key_Egrave"] = 0x0c8;
  m_EventConstMap["Key_Eacute"] = 0x0c9;
  m_EventConstMap["Key_Ecircumflex"] = 0x0ca;
  m_EventConstMap["Key_Ediaeresis"] = 0x0cb;
  m_EventConstMap["Key_Igrave"] = 0x0cc;
  m_EventConstMap["Key_Iacute"] = 0x0cd;
  m_EventConstMap["Key_Icircumflex"] = 0x0ce;
  m_EventConstMap["Key_Idiaeresis"] = 0x0cf;
  m_EventConstMap["Key_ETH"] = 0x0d0;
  m_EventConstMap["Key_Ntilde"] = 0x0d1;
  m_EventConstMap["Key_Ograve"] = 0x0d2;
  m_EventConstMap["Key_Oacute"] = 0x0d3;
  m_EventConstMap["Key_Ocircumflex"] = 0x0d4;
  m_EventConstMap["Key_Otilde"] = 0x0d5;
  m_EventConstMap["Key_Odiaeresis"] = 0x0d6;
  m_EventConstMap["Key_multiply"] = 0x0d7;
  m_EventConstMap["Key_Ooblique"] = 0x0d8;
  m_EventConstMap["Key_Ugrave"] = 0x0d9;
  m_EventConstMap["Key_Uacute"] = 0x0da;
  m_EventConstMap["Key_Ucircumflex"] = 0x0db;
  m_EventConstMap["Key_Udiaeresis"] = 0x0dc;
  m_EventConstMap["Key_Yacute"] = 0x0dd;
  m_EventConstMap["Key_THORN"] = 0x0de;
  m_EventConstMap["Key_ssharp"] = 0x0df;
  m_EventConstMap["Key_agrave"] = 0x0e0;
  m_EventConstMap["Key_aacute"] = 0x0e1;
  m_EventConstMap["Key_acircumflex"] = 0x0e2;
  m_EventConstMap["Key_atilde"] = 0x0e3;
  m_EventConstMap["Key_adiaeresis"] = 0x0e4;
  m_EventConstMap["Key_aring"] = 0x0e5;
  m_EventConstMap["Key_ae"] = 0x0e6;
  m_EventConstMap["Key_ccedilla"] = 0x0e7;
  m_EventConstMap["Key_egrave"] = 0x0e8;
  m_EventConstMap["Key_eacute"] = 0x0e9;
  m_EventConstMap["Key_ecircumflex"] = 0x0ea;
  m_EventConstMap["Key_ediaeresis"] = 0x0eb;
  m_EventConstMap["Key_igrave"] = 0x0ec;
  m_EventConstMap["Key_iacute"] = 0x0ed;
  m_EventConstMap["Key_icircumflex"] = 0x0ee;
  m_EventConstMap["Key_idiaeresis"] = 0x0ef;
  m_EventConstMap["Key_eth"] = 0x0f0;
  m_EventConstMap["Key_ntilde"] = 0x0f1;
  m_EventConstMap["Key_ograve"] = 0x0f2;
  m_EventConstMap["Key_oacute"] = 0x0f3;
  m_EventConstMap["Key_ocircumflex"] = 0x0f4;
  m_EventConstMap["Key_otilde"] = 0x0f5;
  m_EventConstMap["Key_odiaeresis"] = 0x0f6;
  m_EventConstMap["Key_division"] = 0x0f7;
  m_EventConstMap["Key_oslash"] = 0x0f8;
  m_EventConstMap["Key_ugrave"] = 0x0f9;
  m_EventConstMap["Key_uacute"] = 0x0fa;
  m_EventConstMap["Key_ucircumflex"] = 0x0fb;
  m_EventConstMap["Key_udiaeresis"] = 0x0fc;
  m_EventConstMap["Key_yacute"] = 0x0fd;
  m_EventConstMap["Key_thorn"] = 0x0fe;
  m_EventConstMap["Key_ydiaeresis"] = 0x0ff;
  m_EventConstMap["Key_unknown"] = 0xffff;
  m_EventConstMap["Key_none"] = 0xffff;

}

mitk::EventMapper::~EventMapper()
{
}

//##Documentation
//## set the global StateMachine. If temporaryly changed,
//## then copy the old statemachine with GetStateMachine()
void mitk::EventMapper::SetGlobalStateMachine(StateMachine *stateMachine)
{
  itkGenericOutputMacro(<< "Use of mitk::EventMapper::SetGlobalStateMachine. "
    << "This method may be important in the future. Has the future "
    << "already begun?");
  if((s_GlobalStateMachine != NULL) && (s_GlobalStateMachine != stateMachine))
  {
    itkGenericOutputMacro(<< "replacing an existing global state machine");
  }
  s_GlobalStateMachine = stateMachine;
}

mitk::StateMachine* mitk::EventMapper::GetGlobalStateMachine()
{
  if(s_GlobalStateMachine == NULL)
  {
    return mitk::GlobalInteraction::GetInstance();
  }
  return s_GlobalStateMachine;
}

//##Documentation
//## searches the Event in m_EventDescription
//##  
bool mitk::EventMapper::MapEvent(Event* event, int mitkPostedEventID )
{
  int eventID = mitkPostedEventID;

  if (GetGlobalStateMachine() == NULL)
    return false;

  if( mitkPostedEventID == 0 )
  {
    //search the event in the list of event descriptions, if found, then take the number and produce a stateevent
    EventDescriptionVecIter iter;
    for (iter = m_EventDescriptions.begin(); iter!=m_EventDescriptions.end();iter++)
    {
      if (*iter == *event)
        break;
    }

    if (iter == m_EventDescriptions.end())//not found
      return false;
    eventID = (*iter).GetId();
  }

  //set the Menger_Var m_StateEvent and send to StateMachine, which does everything further!
  m_StateEvent.Set( eventID, event );
  /*
  Group and Object EventId:
  then EventMapper has the power to decide which operations hang together;
  each event causes n (n e N) operations (e.g. StateChanges, data-operations...).
  Undo must recall all these coherent operations, so all of the same objectId.
  But Undo has also the power to recall more operationsets, for example a set for building up a new object,
  so that a newly build up object is deleted after a Undo and not only the latest set point.
  The StateMachines::ExecuteAction have the power to descide weather a new GroupID has to be calculated
  (by example after the editing of a new object)

  A user interaction with the mouse is started by a mousePressEvent, continues with a MouseMove and finishes with a MouseReleaseEvent
  */
  bool ok;

  switch (event->GetType())
  {
  case mitk::Type_MouseButtonPress://Increase
    mitk::OperationEvent::IncCurrObjectEventId();
    break;
  case mitk::Type_MouseMove://same
    break;
  case mitk::Type_MouseButtonRelease://same
    break;
  case mitk::Type_User://same
    break;
  case mitk::Type_KeyPress://Increase
    mitk::OperationEvent::IncCurrObjectEventId();
    break;
  default://increase
    mitk::OperationEvent::IncCurrObjectEventId();
  }

#ifdef MBI_INTERNAL_CONFERENCE
  //Conference - pass local events through 
  if ( mitkPostedEventID == 0 )
  {
    mitk::CoreObjectFactory::GetInstance()->MapEvent(event,eventID);
 }
#endif //MBI_INTERNAL_CONFERENCE

  mitk::OperationEvent::ExecuteIncrement();
  ok = GetGlobalStateMachine()->HandleEvent(&m_StateEvent);

  return ok;
}

//##Documentation
//## loads an XML-File containing Events into m_EventDescriptions
//## also involved: EventMapper::startEvent(...)
bool mitk::EventMapper::LoadBehavior(std::string fileName)
{
  if ( fileName.empty() )
    return false;

  mitk::EventMapper* eventMapper = EventMapper::New();
  eventMapper->SetFileName( fileName.c_str() );

  if ( ! eventMapper->Parse() )
  {
#ifdef INTERDEBUG
    //itkWarningMacro("mitk::EventMapper::LoadBehavior xml file cannot parse!" );
    //static function LoadBehavior has no this pointer!
    LOG_INFO<<"mitk::EventMapper::LoadBehavior xml file cannot parse!"<<std::endl;
#endif
  }

  eventMapper->Delete();
  return true;
}

bool mitk::EventMapper::LoadStandardBehavior()
{
  // Search for StateMachine.xml, bypass relative path in mitkSourceTree for additional search
  std::string xmlFileName = mitk::StandardFileLocations::GetInstance()->FindFile("StateMachine.xml", "Core/Interactions");
  if(itksys::SystemTools::FileExists(xmlFileName.c_str()))
    return LoadBehavior(xmlFileName);

  return false;
}

//##Documentation
//## @brief converts the given const String declared in the xml-file
//## to the defined const int
inline int mitk::EventMapper::convertConstString2ConstInt(std::string input)
{
  ConstMapIter tempIt  = m_EventConstMap.find(input.c_str());
  if (tempIt != m_EventConstMap.end())
  {
    return (tempIt)->second;
  }

  //mitk::StatusBar::GetInstance()->DisplayText("Warning! from mitkEventMapper.cpp: Couldn't find matching Event Int from Event String in XML-File");
  return -1;//for didn't find anything
}

void  mitk::EventMapper::StartElement (const char *elementName, const char **atts)
{

  if ( elementName == EVENT )
  {

    // EventDescription(int type, int button, int buttonState,int key, std::string name, int id)
    EventDescription eventDescr( convertConstString2ConstInt( ReadXMLStringAttribut( TYPE, atts )),
      convertConstString2ConstInt( ReadXMLStringAttribut( BUTTON, atts )),
      ReadXMLIntegerAttribut( BUTTONSTATE, atts ),
      convertConstString2ConstInt( ReadXMLStringAttribut( KEY, atts )),
      ReadXMLStringAttribut( NAME, atts ),
      ReadXMLIntegerAttribut( ID, atts ));

    m_EventDescriptions.push_back(eventDescr);
  }
  else if ( elementName == EVENTS )
    m_StyleName = ReadXMLStringAttribut( STYLE, atts );

}

std::string mitk::EventMapper::GetStyleName()
{
  return m_StyleName;
}


std::string mitk::EventMapper::ReadXMLStringAttribut( std::string name, const char** atts )
{
  if(atts)
  {
    const char** attsIter = atts;

    while(*attsIter)
    {
      if ( name == *attsIter )
      {
        attsIter++;
        return *attsIter;
      }
      attsIter++;
      attsIter++;
    }
  }

  return std::string();
}


int mitk::EventMapper::ReadXMLIntegerAttribut( std::string name, const char** atts )
{
  std::string s = ReadXMLStringAttribut( name, atts );
  static const std::string hex = "0x";
  int result;

  if ( s[0] == hex[0] && s[1] == hex[1] )
    result = strtol( s.c_str(), NULL, 16 );
  else
    result = atoi( s.c_str() );

  return result;
}

void mitk::EventMapper::SetStateEvent(mitk::Event* event)
{
  m_StateEvent.Set( m_StateEvent.GetId(), event );
}

mitk::StateEvent* mitk::EventMapper::RefreshStateEvent(mitk::StateEvent* stateEvent)
{
  //search the event in the list of event descriptions, if found, then change it and return the pointer
  EventDescriptionVecIter iter;
  for (iter = m_EventDescriptions.begin(); iter!=m_EventDescriptions.end(); iter++)
  {
    if (*iter == *(stateEvent->GetEvent()))
      break;
  }

  if (iter != m_EventDescriptions.end())//found
  {
    stateEvent->Set((*iter).GetId(), stateEvent->GetEvent());
  }

  return stateEvent;
}

