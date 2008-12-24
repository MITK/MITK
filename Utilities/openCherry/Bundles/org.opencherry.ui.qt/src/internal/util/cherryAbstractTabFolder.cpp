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

#include "cherryAbstractTabFolder.h"
#include "cherryAbstractTabItem.h"

#include <cherryConstants.h>
#include <presentations/cherryIStackPresentationSite.h>

namespace cherry
{

AbstractTabFolder::AbstractTabFolder() :
  activeState(IStackPresentationSite::STATE_RESTORED)
{

}

void AbstractTabFolder::SetActive(int activeState)
{
  this->activeState = activeState;
}

int AbstractTabFolder::GetActive()
{
  return activeState;
}

QPoint AbstractTabFolder::GetPaneMenuLocation()
{
  return this->GetControl()->mapToGlobal(QPoint(0, 0));
}

QPoint AbstractTabFolder::GetPartListLocation()
{
  return this->GetSystemMenuLocation();
}

QPoint AbstractTabFolder::GetSystemMenuLocation()
{
  return this->GetControl()->mapToGlobal(QPoint(0, 0));
}

AbstractTabItem* AbstractTabFolder::GetItem(int idx)
{
  return this->GetItems()[idx];
}

AbstractTabItem* AbstractTabFolder::GetItem(const QPoint& toFind)
{
  std::vector<AbstractTabItem*> items = this->GetItems();

  for (unsigned int i = 0; i < items.size(); i++)
  {
    AbstractTabItem* item = items[i];

    if (item->GetBounds().contains(toFind))
    {
      return item;
    }
  }

  return 0;
}

AbstractTabItem* AbstractTabFolder::FindItem(Object::Pointer dataToFind)
{
  std::vector<AbstractTabItem*> items = this->GetItems();

  for (unsigned int i = 0; i < items.size(); i++)
  {
    AbstractTabItem* item = items[i];

    if (item->GetData() == dataToFind)
    {
      return item;
    }
  }

  return 0;
}

int AbstractTabFolder::IndexOf(AbstractTabItem* item)
{
  std::vector<AbstractTabItem*> items = this->GetItems();

  for (unsigned int idx = 0; idx < items.size(); ++idx)
  {
    AbstractTabItem* next = items[idx];

    if (next == item)
    {
      return idx;
    }
  }

  return -1;
}

int AbstractTabFolder::GetItemCount()
{
  return this->GetItems().size();
}

void AbstractTabFolder::SetState(int state)
{
  this->state = state;
}

void AbstractTabFolder::ShellActive(bool isActive)
{
}

void AbstractTabFolder::Layout(bool flushCache)
{
}

void AbstractTabFolder::SetTabPosition(int tabPosition)
{
}

int AbstractTabFolder::GetTabPosition()
{
  return Constants::TOP;
}

int AbstractTabFolder::GetState()
{
  return state;
}

bool AbstractTabFolder::IsOnBorder(const QPoint& toTest)
{
  return false;
}

void AbstractTabFolder::SetVisible(bool visible)
{
  this->GetControl()->setVisible(visible);
}

void AbstractTabFolder::ShowMinMax(bool show)
{
}

void AbstractTabFolder::FireEvent(TabFolderEvent::Pointer e)
{
  tabFolderEvent(e);
}

void AbstractTabFolder::FireEvent(int id)
{
  TabFolderEvent::Pointer event = new TabFolderEvent(id);
  this->FireEvent(event);
}

void AbstractTabFolder::FireEvent(int id, AbstractTabItem* w)
{
  TabFolderEvent::Pointer event = new TabFolderEvent(id, w, 0, 0);
  this->FireEvent(event);
}

void AbstractTabFolder::FireEvent(int id, AbstractTabItem* w, const QPoint& pos)
{
  TabFolderEvent::Pointer event = new TabFolderEvent(id, w, pos);
  this->FireEvent(event);
}

void AbstractTabFolder::HandleContextMenu(const QPoint& displayPos)
{
  if (this->IsOnBorder(displayPos))
  {
    return;
  }

  AbstractTabItem* tab = this->GetItem(displayPos);

  this->FireEvent(TabFolderEvent::EVENT_SYSTEM_MENU, tab, displayPos);
}

void AbstractTabFolder::HandleMouseDown(const QPoint& displayPos)
{
  this->FireEvent(TabFolderEvent::EVENT_GIVE_FOCUS_TO_PART);
}

void AbstractTabFolder::HandleDoubleClick(const QPoint& displayPos)
{
  if (this->IsOnBorder(displayPos))
  {
    return;
  }

  if (this->GetState() == IStackPresentationSite::STATE_MAXIMIZED)
  {
    this->FireEvent(TabFolderEvent::EVENT_RESTORE);
  }
  else
  {
    this->FireEvent(TabFolderEvent::EVENT_MAXIMIZE);
  }
}

void AbstractTabFolder::HandleDragStarted(const QPoint& displayPos)
{

  if (this->IsOnBorder(displayPos))
  {
    return;
  }

  AbstractTabItem* tab = this->GetItem(displayPos);
  this->FireEvent(TabFolderEvent::EVENT_DRAG_START, tab, displayPos);
}

}
