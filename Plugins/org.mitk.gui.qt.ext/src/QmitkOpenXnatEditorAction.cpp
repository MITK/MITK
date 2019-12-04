/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkOpenXnatEditorAction.h"

#include <berryIEditorPart.h>
#include <berryIWorkbenchPage.h>
#include <berryIPreferencesService.h>
#include <berryIWorkbench.h>
#include <berryPlatform.h>
#include <berryFileEditorInput.h>

QmitkOpenXnatEditorAction::QmitkOpenXnatEditorAction(berry::IWorkbenchWindow::Pointer window)
: QAction(nullptr)
{
  this->init(window);
}

QmitkOpenXnatEditorAction::QmitkOpenXnatEditorAction(const QIcon & icon, berry::IWorkbenchWindow::Pointer window)
: QAction(nullptr)
{
  this->setIcon(icon);

  this->init(window);
}

void QmitkOpenXnatEditorAction::init(berry::IWorkbenchWindow::Pointer window)
{
  m_Window = window;
  this->setParent(static_cast<QWidget*>(m_Window->GetShell()->GetControl()));
  this->setText("&XNAT");
  this->setToolTip("Open XNAT tool");

  berry::IPreferencesService* prefService = berry::Platform::GetPreferencesService();

  m_GeneralPreferencesNode = prefService->GetSystemPreferences()->Node("/General");

  this->connect(this, SIGNAL(triggered(bool)), this, SLOT(Run()));
}

void QmitkOpenXnatEditorAction::Run()
{
  // check if there is an open perspective, if not open the default perspective
  if (m_Window->GetActivePage().IsNull())
  {
    QString defaultPerspId = m_Window->GetWorkbench()->GetPerspectiveRegistry()->GetDefaultPerspective();
    m_Window->GetWorkbench()->ShowPerspective(defaultPerspId, m_Window);
  }

  QList<berry::IEditorReference::Pointer> editors =
    m_Window->GetActivePage()->FindEditors(berry::IEditorInput::Pointer(nullptr),
    "org.mitk.editors.xnat.browser", berry::IWorkbenchPage::MATCH_ID);

  if (editors.empty())
  {
    // no XnatEditor is currently open, create a new one
    berry::IEditorInput::Pointer editorInput(new berry::FileEditorInput(QString()));
    m_Window->GetActivePage()->OpenEditor(editorInput, "org.mitk.editors.xnat.browser");
  }
  else
  {
    // reuse an existing editor
    berry::IEditorPart::Pointer reuseEditor = editors.front()->GetEditor(true);
    m_Window->GetActivePage()->Activate(reuseEditor);
  }
}
