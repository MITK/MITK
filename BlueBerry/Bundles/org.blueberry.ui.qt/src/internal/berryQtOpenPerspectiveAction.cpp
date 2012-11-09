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

#include "berryQtOpenPerspectiveAction.h"

#include <berryIWorkbenchPage.h>
#include <berryIWorkbench.h>

#include <QWidget>
#include <QMessageBox>

namespace berry
{

QtOpenPerspectiveAction::QtOpenPerspectiveAction(
    IWorkbenchWindow::Pointer window, IPerspectiveDescriptor::Pointer descr, QActionGroup* group) :
  QAction(0),
  window(window.GetPointer())
{
  this->setParent(group);
  this->setText(QString(descr->GetLabel().c_str()));
  this->setToolTip(QString(descr->GetLabel().c_str()));
  this->setCheckable(true);
  this->setIconVisibleInMenu(true);

  group->addAction(this);

  QIcon* icon = static_cast<QIcon*>(descr->GetImageDescriptor()->CreateImage());
  this->setIcon(*icon);
  descr->GetImageDescriptor()->DestroyImage(icon);

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
    QMessageBox::critical(0, "Opening Perspective Failed", QString("The perspective \"") + this->text() + "\" could not be opened.\nSee the log for details.");
  }
}

}
