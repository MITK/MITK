/*=========================================================================

 Program:   openCherry Platform
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

#ifndef CHERRYTABFOLDEREVENT_H_
#define CHERRYTABFOLDEREVENT_H_

#include <presentations/cherryIStackPresentationSite.h>

#include <QPoint>

namespace cherry
{

class AbstractTabItem;

struct TabFolderEvent : public Object
{
  cherryObjectMacro(TabFolderEvent);

  static const int EVENT_PANE_MENU; // = 1;
  static const int EVENT_HIDE_TOOLBAR; // = 2;
  static const int EVENT_SHOW_TOOLBAR; // = 3;
  static const int EVENT_RESTORE; // = 4;
  static const int EVENT_MINIMIZE; // = 5;
  static const int EVENT_CLOSE; // = 6;
  static const int EVENT_MAXIMIZE; // = 7;
  static const int EVENT_TAB_SELECTED; // = 8;
  static const int EVENT_GIVE_FOCUS_TO_PART; // = 9;
  static const int EVENT_DRAG_START; // = 10;
  static const int EVENT_SHOW_LIST; // = 11;
  static const int EVENT_SYSTEM_MENU; // = 12;
  static const int EVENT_PREFERRED_SIZE; // = 13;

  static int EventIdToStackState(int eventId);

  static int StackStateToEventId(int stackState);

  AbstractTabItem* tab;
  int type;
  int x;
  int y;

  TabFolderEvent(int _type);

  TabFolderEvent(int type, AbstractTabItem* w, int x, int y);

  TabFolderEvent(int type, AbstractTabItem* w, const QPoint& pos);
};

}

#endif /* CHERRYTABFOLDEREVENT_H_ */
