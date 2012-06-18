/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2010-01-16 19:57:43 +0100 (Sa, 16 Jan 2010) $
Version:   $Revision: 21070 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "QmitkOpenDicomEditorAction.h"

#include <QFileDialog>
#include <QFileInfo>
#include <mitkDataNodeFactory.h>

#include "mitkCoreObjectFactory.h"
#include "mitkSceneIO.h"
#include "mitkProgressBar.h"

#include <mitkCoreExtObjectFactory.h>
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


//#include <QmitkStdMultiWidgetEditor.h>

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

  berry::IPreferencesService::Pointer prefService
    = berry::Platform::GetServiceRegistry()
    .GetServiceById<berry::IPreferencesService>(berry::IPreferencesService::ID);
  
  m_GeneralPreferencesNode = prefService->GetSystemPreferences()->Node("/General");

  this->connect(this, SIGNAL(triggered(bool)), this, SLOT(Run()));
}

void QmitkOpenDicomEditorAction::Run()
{

 // check if there is an open perspective, if not open the default perspective
    if (m_Window->GetActivePage().IsNull())
    {
        std::string defaultPerspId = m_Window->GetWorkbench()->GetPerspectiveRegistry()->GetDefaultPerspective();
        m_Window->GetWorkbench()->ShowPerspective(defaultPerspId, m_Window);
    }

    mitk::DataStorageEditorInput::Pointer editorInput;
    //mitk::DataStorage::Pointer dataStorage;
    //QmitkStdMultiWidgetEditor::Pointer multiWidgetEditor;
    //berry::IEditorPart::Pointer editor = m_Window->GetActivePage()->GetActiveEditor();
   

   
    //if (editor.Cast<QmitkStdMultiWidgetEditor>().IsNull())
    //{
    //    editorInput = new mitk::DataStorageEditorInput();
    //    dataStorage = editorInput->GetDataStorageReference()->GetDataStorage();
    //}
    //else
    //{
    //    multiWidgetEditor = editor.Cast<QmitkStdMultiWidgetEditor>();
    //    dataStorage = multiWidgetEditor->GetEditorInput().Cast<mitk::DataStorageEditorInput>()->GetDataStorageReference()->GetDataStorage();
    //}

    //if (multiWidgetEditor.IsNull())
    //{
    //    //berry::IEditorPart::Pointer editor = m_Window->GetActivePage()->OpenEditor(editorInput, QmitkStdMultiWidgetEditor::EDITOR_ID);
    //    multiWidgetEditor = editor.Cast<QmitkStdMultiWidgetEditor>();
    //}
    //else
    //{
    //    multiWidgetEditor->GetStdMultiWidget()->RequestUpdate();
    //}

    berry::IEditorInput::Pointer editorInput2(new berry::FileEditorInput(Poco::Path()));
    m_Window->GetActivePage()->OpenEditor(editorInput2, "org.mitk.editors.dicomeditor");
}

