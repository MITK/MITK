/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "berryQtPerspectiveSwitcher.h"

#include "berryQtOpenPerspectiveAction.h"
#include <berryIWorkbench.h>
#include <berryIWorkbenchPage.h>
#include <berryIPerspectiveRegistry.h>

#include <QActionGroup>

namespace berry {

struct QtPerspectiveSwitcherListener : public IPerspectiveListener
{
  QtPerspectiveSwitcherListener(QtPerspectiveSwitcher* switcher)
  : switcher(switcher)
  {}

  Events::Types GetPerspectiveEventTypes() const
  {
    return Events::ACTIVATED;
  }

  void PerspectiveActivated(const IWorkbenchPage::Pointer& /*page*/,
                            const IPerspectiveDescriptor::Pointer& perspective)
  {
    QAction* action = switcher->perspIdToActionMap[perspective->GetId()];
    if (action) action->setChecked(true);
  }

private:

  QtPerspectiveSwitcher* switcher;
};


QtPerspectiveSwitcher::QtPerspectiveSwitcher(IWorkbenchWindow::Pointer window)
: window(window)
{
  QWidget* parent = static_cast<QWidget*>(window->GetShell()->GetControl());
  this->setParent(parent);

  this->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

  QActionGroup* perspGroup = new QActionGroup(this);

  IPerspectiveRegistry* perspRegistry = window->GetWorkbench()->GetPerspectiveRegistry();

  QList<IPerspectiveDescriptor::Pointer> perspectives(perspRegistry->GetPerspectives());
  for (QList<IPerspectiveDescriptor::Pointer>::iterator perspIt =
      perspectives.begin(); perspIt != perspectives.end(); ++perspIt)
  {
    QAction* perspAction = new QtOpenPerspectiveAction(window, *perspIt, perspGroup);
    perspIdToActionMap[(*perspIt)->GetId()] = perspAction;
  }
  this->addActions(perspGroup->actions());

  perspListener.reset(new QtPerspectiveSwitcherListener(this));
  window->AddPerspectiveListener(perspListener.data());
}

QtPerspectiveSwitcher::~QtPerspectiveSwitcher()
{
  window->RemovePerspectiveListener(perspListener.data());
}

}
