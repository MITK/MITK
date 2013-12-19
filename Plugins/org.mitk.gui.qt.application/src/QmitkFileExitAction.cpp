/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "QmitkFileExitAction.h"

#include <berryPlatformUI.h>

QmitkFileExitAction::QmitkFileExitAction(berry::IWorkbenchWindow::Pointer window)
: QAction(0)
{
  m_Window = window.GetPointer();

  this->setParent(static_cast<QWidget*>(m_Window->GetShell()->GetControl()));
  this->setText("&Exit");

  this->connect(this, SIGNAL(triggered(bool)), this, SLOT(Run()));
}

void QmitkFileExitAction::Run()
{
  berry::PlatformUI::GetWorkbench()->Close();
}
