/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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

  Events::Types GetPerspectiveEventTypes() const override
  {
    return Events::ACTIVATED;
  }

  void PerspectiveActivated(const IWorkbenchPage::Pointer& /*page*/,
                            const IPerspectiveDescriptor::Pointer& perspective) override
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

  auto   perspGroup = new QActionGroup(this);

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
