/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryQtOpenPerspectiveAction.h"

#include <berryIWorkbenchPage.h>
#include <berryIWorkbench.h>

#include <QWidget>
#include <QMessageBox>

namespace berry
{

QtOpenPerspectiveAction::QtOpenPerspectiveAction(
    IWorkbenchWindow::Pointer window, IPerspectiveDescriptor::Pointer descr, QActionGroup* group) :
  QAction(nullptr),
  window(window.GetPointer())
{
  this->setParent(group);
  this->setText(descr->GetLabel());
  this->setToolTip(descr->GetLabel());
  this->setCheckable(true);
  this->setIconVisibleInMenu(true);

  group->addAction(this);

  QIcon icon = descr->GetImageDescriptor();
  this->setIcon(icon);

  perspectiveId = descr->GetId();

  this->connect(this, SIGNAL(triggered(bool)), this, SLOT(Run()));
}

void QtOpenPerspectiveAction::Run()
{
  try
  {
    window->GetWorkbench()->ShowPerspective(perspectiveId, IWorkbenchWindow::Pointer(window));
  }
  catch (...)
  {
    QMessageBox::critical(nullptr, "Opening Perspective Failed", QString("The perspective \"") + this->text() + "\" could not be opened.\nSee the log for details.");
  }
}

}
