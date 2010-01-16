/*=========================================================================
 
 Program:   BlueBerry Platform
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

#include "berryQtOpenPerspectiveAction.h"

#include <berryIWorkbenchPage.h>

#include <QWidget>
#include <QMessageBox>

namespace berry
{

QtOpenPerspectiveAction::QtOpenPerspectiveAction(
    IWorkbenchWindow::Pointer window, IPerspectiveDescriptor::Pointer descr, QActionGroup* group) :
  QAction(0),
  window(window.GetPointer()), descr(descr)
{
  this->setParent(group);
  this->setText(QString(descr->GetLabel().c_str()));
  this->setToolTip(QString(descr->GetLabel().c_str()));
  this->setCheckable(true);

  group->addAction(this);

  QIcon* icon = static_cast<QIcon*>(descr->GetImageDescriptor()->CreateImage());
  this->setIcon(*icon);
  descr->GetImageDescriptor()->DestroyImage(icon);

  this->connect(this, SIGNAL(triggered(bool)), this, SLOT(Run()));
}

void QtOpenPerspectiveAction::Run()
{
  try
  {
    if (window->GetActivePage()->GetPerspective() != descr)
    {
      window->GetActivePage()->SetPerspective(descr);
    }
  }
  catch (...)
  {
    QMessageBox::critical(0, "Opening Perspective Failed", QString::fromStdString("The perspective \"" + descr->GetLabel() + "\" could not be opened.\nSee the log for details."));
  }
}

}
