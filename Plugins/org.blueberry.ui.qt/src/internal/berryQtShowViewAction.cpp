/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryQtShowViewAction.h"

#include <berryIWorkbenchPage.h>
#include <berryUIException.h>

namespace berry
{

QtShowViewAction::QtShowViewAction(IWorkbenchWindow::Pointer window,
    IViewDescriptor::Pointer desc) :
  QAction(nullptr)
{
  this->setParent(static_cast<QWidget*>(window->GetShell()->GetControl()));
  this->setText(desc->GetLabel());
  this->setToolTip(desc->GetLabel());
  this->setIconVisibleInMenu(true);

  QIcon icon = desc->GetImageDescriptor();
  this->setIcon(icon);

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
    catch (const PartInitException& e)
    {
      BERRY_ERROR << "Error: " << e.what();
    }
  }
}

}
