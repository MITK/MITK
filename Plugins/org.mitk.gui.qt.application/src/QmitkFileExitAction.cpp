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
#include "internal/org_mitk_gui_qt_application_Activator.h"

#include <berryPlatformUI.h>

QmitkFileExitAction::QmitkFileExitAction(berry::IWorkbenchWindow::Pointer window)
  : QAction(nullptr)
  , m_Window(nullptr)
{
  this->init(window.GetPointer());
}

QmitkFileExitAction::QmitkFileExitAction(const QIcon & icon, berry::IWorkbenchWindow::Pointer window)
  : QAction(nullptr)
  , m_Window(nullptr)
{
  this->setIcon(icon);
  this->init(window.GetPointer());
}

QmitkFileExitAction::QmitkFileExitAction(berry::IWorkbenchWindow* window)
  : QAction(nullptr)
  , m_Window(nullptr)
{
  this->init(window);
}

QmitkFileExitAction::QmitkFileExitAction(const QIcon& icon, berry::IWorkbenchWindow* window)
  : QAction(nullptr)
  , m_Window(nullptr)
{
  this->setIcon(icon);
  this->init(window);
}

void QmitkFileExitAction::init(berry::IWorkbenchWindow* window)
{
  m_Window = window;
  this->setText("&Exit");
  this->setToolTip("Exit the application. Please save your data before exiting.");
  this->connect(this, SIGNAL(triggered(bool)), this, SLOT(Run()));
}

void QmitkFileExitAction::Run()
{
  berry::PlatformUI::GetWorkbench()->Close();
}
