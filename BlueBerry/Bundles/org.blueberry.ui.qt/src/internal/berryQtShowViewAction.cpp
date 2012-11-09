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

#include "berryQtShowViewAction.h"

#include <berryIWorkbenchPage.h>
#include <berryUIException.h>

namespace berry
{

QtShowViewAction::QtShowViewAction(IWorkbenchWindow::Pointer window,
    IViewDescriptor::Pointer desc) :
  QAction(0)
{
  this->setParent(static_cast<QWidget*>(window->GetShell()->GetControl()));
  this->setText(QString(desc->GetLabel().c_str()));
  this->setToolTip(QString(desc->GetLabel().c_str()));
  this->setIconVisibleInMenu(true);

  QIcon* icon = static_cast<QIcon*>(desc->GetImageDescriptor()->CreateImage());
  this->setIcon(*icon);
  desc->GetImageDescriptor()->DestroyImage(icon);

  m_Window = window.GetPointer();
  m_Desc = desc;

  this->connect(this, SIGNAL(triggered(bool)), this, SLOT(Run()));
}

void QtShowViewAction::Run()
{
  IWorkbenchPage::Pointer page = m_Window->GetActivePage();
  if (page.IsNotNull())
  {
    try
    {
      page->ShowView(m_Desc->GetId());
    }
    catch (PartInitException e)
    {
      BERRY_ERROR << "Error: " << e.displayText() << std::endl;
    }
  }
}

}
