/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
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

#include "QmitkFileOpenAction.h"

#include <QFileDialog>
#include <QFileInfo>
#include <mitkDataTreeNodeFactory.h>

#include "mitkProgressBar.h"

#include <mitkCoreObjectFactory.h>
#include <mitkDataStorageEditorInput.h>
#include <berryIEditorPart.h>
#include <berryIWorkbenchPage.h>
#include <berryIPreferencesService.h>
#include <berryPlatform.h>

#include "mitkProperties.h"
#include "mitkNodePredicateData.h"
#include "mitkNodePredicateNOT.h"
#include "mitkNodePredicateProperty.h"


#include "QmitkStdMultiWidgetEditor.h"

QmitkFileOpenAction::QmitkFileOpenAction(berry::IWorkbenchWindow::Pointer window)
: QAction(0)
{
  this->init(window);
}

QmitkFileOpenAction::QmitkFileOpenAction(const QIcon & icon, berry::IWorkbenchWindow::Pointer window)
: QAction(0)
{
  this->setIcon(icon);

  this->init(window);
}

void QmitkFileOpenAction::init(berry::IWorkbenchWindow::Pointer window)
{
  m_Window = window.GetPointer();
  this->setParent(static_cast<QWidget*>(m_Window->GetShell()->GetControl()));
  this->setText("&Open...");
  this->setToolTip("Open data files (images, surfaces,...) and project files (.mitk)");

  berry::IPreferencesService::Pointer prefService
    = berry::Platform::GetServiceRegistry()
    .GetServiceById<berry::IPreferencesService>(berry::IPreferencesService::ID);
  m_GeneralPreferencesNode = prefService->GetSystemPreferences()->Node("/General");

  this->connect(this, SIGNAL(triggered(bool)), this, SLOT(Run()));
}

void QmitkFileOpenAction::Run()
{
  /**
   * @brief stores the last path of last opened file
   */
  static QString m_LastPath;

  if(m_GeneralPreferencesNode.Lock().IsNotNull())
  {
    if(m_LastPath.isEmpty())
      m_LastPath = QString::fromStdString(m_GeneralPreferencesNode.Lock()->Get("LastFileOpenPath", ""));
  }


  //QFileDialog dialog(static_cast<QWidget*>(m_Window->GetShell()->GetControl()));
  //dialog.setFileMode(QFileDialog::ExistingFiles);
  //QStringList filters;
  //filters << "Images (*.pic *.pic.gz *.vti *.dcm *.nhdr *.nrrd *.mhd)" 
  //  << "Surfaces (*.stl *.vtk *.vtp)"
  //  << "MITK Pointset (*.mps)"
  //  << "All Files (*.*)";
  //dialog.setFilters(filters);
  QStringList fileNames = QFileDialog::getOpenFileNames(NULL,"Open",m_LastPath,mitk::CoreObjectFactory::GetInstance()->GetFileExtensions());

  //if (dialog.exec())
  //  fileNames = dialog.selectedFiles();

  if (fileNames.empty()) 
    return;

  QFileInfo info(fileNames.at(0));
  m_LastPath = info.filePath();
  if(m_GeneralPreferencesNode.Lock().IsNotNull())
  {
    m_GeneralPreferencesNode.Lock()->Put("LastFileOpenPath", m_LastPath.toStdString());
    m_GeneralPreferencesNode.Lock()->Flush();
  }

  mitk::DataStorageEditorInput::Pointer editorInput;
  mitk::DataStorage::Pointer dataStorage;
  QmitkStdMultiWidgetEditor::Pointer multiWidgetEditor;
  berry::IEditorPart::Pointer editor = m_Window->GetActivePage()->GetActiveEditor();
  if (editor.Cast<QmitkStdMultiWidgetEditor>().IsNull())
  {
    editorInput = new mitk::DataStorageEditorInput();
    dataStorage = editorInput->GetDataStorageReference()->GetDataStorage();
  }
  else
  {
    multiWidgetEditor = editor.Cast<QmitkStdMultiWidgetEditor>();
    dataStorage = multiWidgetEditor->GetEditorInput().Cast<mitk::DataStorageEditorInput>()->GetDataStorageReference()->GetDataStorage();
  }

  bool dsmodified = false;
  for (QStringList::Iterator fileName = fileNames.begin();
    fileName != fileNames.end(); ++fileName)
  {

    mitk::DataTreeNodeFactory::Pointer nodeReader = mitk::DataTreeNodeFactory::New();
    try
    {
      nodeReader->SetFileName(fileName->toStdString());
      nodeReader->Update();
      for ( unsigned int i = 0 ; i < nodeReader->GetNumberOfOutputs( ); ++i )
      {
        mitk::DataTreeNode::Pointer node;
        node = nodeReader->GetOutput(i);
        if ( node->GetData() != NULL )
        {  
          dataStorage->Add(node);
          dsmodified = true;
        }
      }
    }
    catch(...)
    {

    }

  }
  
  if (multiWidgetEditor.IsNull())
  {
    berry::IEditorPart::Pointer editor = m_Window->GetActivePage()->OpenEditor(editorInput, QmitkStdMultiWidgetEditor::EDITOR_ID);
    multiWidgetEditor = editor.Cast<QmitkStdMultiWidgetEditor>();
  }
  else
  {
    multiWidgetEditor->GetStdMultiWidget()->RequestUpdate();
  }

  if(dsmodified)
  {
    // get all nodes that have not set "includeInBoundingBox" to false
    mitk::NodePredicateNOT::Pointer pred 
      = mitk::NodePredicateNOT::New(mitk::NodePredicateProperty::New("includeInBoundingBox"
      , mitk::BoolProperty::New(false)));

    mitk::DataStorage::SetOfObjects::ConstPointer rs = dataStorage->GetSubset(pred);
    // calculate bounding geometry of these nodes
    mitk::TimeSlicedGeometry::Pointer bounds = dataStorage->ComputeBoundingGeometry3D(rs);
    // initialize the views to the bounding geometry
    mitk::RenderingManager::GetInstance()->InitializeViews(bounds);
  }

 
}
