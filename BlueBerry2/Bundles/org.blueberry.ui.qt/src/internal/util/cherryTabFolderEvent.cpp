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

#include "cherryTabFolderEvent.h"

#include "cherryAbstractTabItem.h"

namespace cherry
{

const int TabFolderEvent::EVENT_PANE_MENU = 1;
const int TabFolderEvent::EVENT_HIDE_TOOLBAR = 2;
const int TabFolderEvent::EVENT_SHOW_TOOLBAR = 3;
const int TabFolderEvent::EVENT_RESTORE = 4;
const int TabFolderEvent::EVENT_MINIMIZE = 5;
const int TabFolderEvent::EVENT_CLOSE = 6;
const int TabFolderEvent::EVENT_MAXIMIZE = 7;
const int TabFolderEvent::EVENT_TAB_SELECTED = 8;
const int TabFolderEvent::EVENT_GIVE_FOCUS_TO_PART = 9;
const int TabFolderEvent::EVENT_DRAG_START = 10;
const int TabFolderEvent::EVENT_SHOW_LIST = 11;
const int TabFolderEvent::EVENT_SYSTEM_MENU = 12;
const int TabFolderEvent::EVENT_PREFERRED_SIZE = 13;

int TabFolderEvent::EventIdToStackState(int eventId)
{
  switch (eventId)
  {
  case EVENT_RESTORE:
    return IStackPresentationSite::STATE_RESTORED;
  case EVENT_MINIMIZE:
    return IStackPresentationSite::STATE_MINIMIZED;
  case EVENT_MAXIMIZE:
    return IStackPresentationSite::STATE_MAXIMIZED;
  }

  return 0;
}

int TabFolderEvent::StackStateToEventId(int stackState)
{
  if (stackState == IStackPresentationSite::STATE_RESTORED)
    return EVENT_RESTORE;
  else if (stackState == IStackPresentationSite::STATE_MINIMIZED)
    return EVENT_MINIMIZE;
  else if (stackState == IStackPresentationSite::STATE_MAXIMIZED)
    return EVENT_MAXIMIZE;

  return 0;
}

TabFolderEvent::TabFolderEvent(int _type) :
  type(_type)
{

}

TabFolderEvent::TabFolderEvent(int _type, AbstractTabItem* w, int _x, int _y)
 : tab(w), type(_type), x(_x), y(_y)
{

}

TabFolderEvent::TabFolderEvent(int _type, AbstractTabItem* w, const QPoint& pos)
 : tab(w), type(_type), x(pos.x()), y(pos.y())
{

}

}
