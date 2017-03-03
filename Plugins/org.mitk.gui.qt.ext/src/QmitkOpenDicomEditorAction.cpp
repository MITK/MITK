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

#include "QmitkOpenDicomEditorAction.h"

#include <QFileDialog>
#include <QFileInfo>

#include "mitkCoreObjectFactory.h"
#include "mitkSceneIO.h"
#include "mitkProgressBar.h"

#include <mitkDataStorageEditorInput.h>
#include <berryIEditorPart.h>
#include <berryIWorkbenchPage.h>
#include <berryIPreferencesService.h>
#include <berryIWorkbench.h>
#include <berryPlatform.h>
#include <berryFileEditorInput.h>

#include "mitkProperties.h"
#include "mitkNodePredicateData.h"
#include "mitkNodePredicateNot.h"
#include "mitkNodePredicateProperty.h"


QmitkOpenDicomEditorAction::QmitkOpenDicomEditorAction(berry::IWorkbenchWindow::Pointer window)
: QAction(0)
{
  this->init(window);
}

QmitkOpenDicomEditorAction::QmitkOpenDicomEditorAction(const QIcon & icon, berry::IWorkbenchWindow::Pointer window)
: QAction(0)
{
  this->setIcon(icon);

  this->init(window);
}

void QmitkOpenDicomEditorAction::init(berry::IWorkbenchWindow::Pointer window)
{
  m_Window = window;
  this->setParent(static_cast<QWidget*>(m_Window->GetShell()->GetControl()));
  this->setText("&DICOM");
  this->setToolTip("Open dicom tool");

  berry::IPreferencesService* prefService = berry::Platform::GetPreferencesService();

  m_GeneralPreferencesNode = prefService->GetSystemPreferences()->Node("/General");

  this->connect(this, SIGNAL(triggered(bool)), this, SLOT(Run()));
}

void QmitkOpenDicomEditorAction::Run()
{

 // check if there is an open perspective, if not open the default perspective
  if (m_Window->GetActivePage().IsNull())
  {
    QString defaultPerspId = m_Window->GetWorkbench()->GetPerspectiveRegistry()->GetDefaultPerspective();
    m_Window->GetWorkbench()->ShowPerspective(defaultPerspId, m_Window);
  }

  berry::IEditorInput::Pointer editorInput2(new berry::FileEditorInput(QString()));
  m_Window->GetActivePage()->OpenEditor(editorInput2, "org.mitk.editors.dicomeditor");
}

