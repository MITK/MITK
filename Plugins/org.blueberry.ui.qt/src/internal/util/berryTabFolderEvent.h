/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYTABFOLDEREVENT_H_
#define BERRYTABFOLDEREVENT_H_

#include <berryIStackPresentationSite.h>

#include <QPoint>

namespace berry
{

struct AbstractTabItem;

struct TabFolderEvent : public Object
{
  berryObjectMacro(TabFolderEvent);

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

#endif /* BERRYTABFOLDEREVENT_H_ */
