/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berrySwitchToWindowMenu.h"

#include <berryMenuManager.h>
#include <berryShell.h>

#include <berryIWorkbench.h>
#include <berryIWorkbenchWindow.h>

#include <QAction>
#include <QActionGroup>
#include <QMenu>
#include <QSignalMapper>

namespace berry {

void SwitchToWindowMenu::AboutToShow(IMenuManager* manager)
{
  manager->MarkDirty();
  dirty = true;
}

void SwitchToWindowMenu::MenuItemTriggered(int index)
{
  if (index < 0 || index >= windows.size()) return;

  IWorkbenchWindow* window = windows[index];
  Shell::Pointer windowShell = window->GetShell();
  if (windowShell->GetMinimized())
  {
    windowShell->SetMinimized(false);
  }
  windowShell->SetActive();
  windowShell->GetControl()->raise();

}

QString SwitchToWindowMenu::CalcText(int number, IWorkbenchWindow* window)
{
  QString suffix = window->GetShell()->GetText();
  if (suffix.isEmpty())
  {
    return QString::null;
  }

  QString sb;
  if (number < 10)
  {
    sb.append('&');
  }
  sb.append(QString::number(number));
  sb.append(' ');
  if (suffix.size() <= MAX_TEXT_LENGTH)
  {
    sb.append(suffix);
  }
  else
  {
    sb.append(suffix.left(MAX_TEXT_LENGTH));
    sb.append("...");
  }
  return sb;
}

SwitchToWindowMenu::SwitchToWindowMenu(IWorkbenchWindow* window, const QString& id, bool showSeparator)
  : ContributionItem(id)
  , workbenchWindow(window)
  , showSeparator(showSeparator)
  , dirty(true)
{
}

void SwitchToWindowMenu::Fill(QMenu* menu, QAction* before)
{

  // Get workbench windows.
  IWorkbench* workbench = workbenchWindow->GetWorkbench();
  windows.clear();
  for (auto window : workbench->GetWorkbenchWindows())
  {
    windows.push_back(window.GetPointer());
  }

  // avoid showing the separator and list for 0 or 1 items
  if (windows.size() < 2)
  {
    return;
  }

  if (MenuManager* mm = dynamic_cast<MenuManager*>(GetParent()))
  {
    this->connect(mm, SIGNAL(AboutToShow(IMenuManager*)), SLOT(AboutToShow(IMenuManager*)));
  }

  if (!dirty)
  {
    return;
  }

  // Add separator.
  if (showSeparator)
  {
    menu->insertSeparator(before);
  }

  // Add one item for each window.
  auto   actionGroup = new QActionGroup(menu);
  actionGroup->setExclusive(true);
  auto   signalMapper = new QSignalMapper(menu);
  connect(signalMapper, SIGNAL(mapped(int)), SLOT(MenuItemTriggered(int)));
  int count = 0;
  for (auto window : windows)
  {
    // can encounter disposed shells if this update is in response to a shell closing
    //if (!window->GetShell()->IsDisposed())
    //{
      QString name = CalcText(count, window);
      if (!name.isEmpty())
      {
        auto   mi = new QAction(name, menu);
        mi->setCheckable(true);
        mi->setChecked(window == workbenchWindow);
        actionGroup->addAction(mi);
        menu->insertAction(before, mi);
        signalMapper->setMapping(mi, count);
        connect(mi, SIGNAL(triggered()), signalMapper, SLOT(map()));
      }
      ++count;
    }
  //}
  dirty = false;
}

bool SwitchToWindowMenu::IsDirty() const
{
  return dirty;
}

bool SwitchToWindowMenu::IsDynamic() const
{
  return true;
}

}
