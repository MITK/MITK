/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-08-26 18:11:29 +0200 (Mi, 26 Aug 2009) $
Version:   $Revision: 18726 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "QmitkFileExitAction.h"

#include <QFileDialog>
#include <mitkDataTreeNodeFactory.h>

#include <mitkCoreObjectFactory.h>
#include <mitkDataStorageEditorInput.h>
#include <berryIEditorPart.h>
#include <berryIWorkbenchPage.h>

#include "QmitkStdMultiWidgetEditor.h"
#include <berryPlatformUI.h>

QmitkFileExitAction::QmitkFileExitAction(berry::IWorkbenchWindow::Pointer window)
: QAction(0)
{
  m_Window = window;
  this->setParent(static_cast<QWidget*>(m_Window->GetShell()->GetControl()));
  this->setText("&Exit");

  m_Window = window;

  this->connect(this, SIGNAL(triggered(bool)), this, SLOT(Run()));
}

void QmitkFileExitAction::Run()
{
  berry::PlatformUI::GetWorkbench()->Close();
}
