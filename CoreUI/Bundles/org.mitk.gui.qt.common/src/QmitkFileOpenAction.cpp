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

#include "mitkSceneIO.h"
#include "mitkProgressBar.h"

#include <mitkCoreObjectFactory.h>
#include <mitkDataStorageEditorInput.h>
#include <cherryIEditorPart.h>
#include <cherryIWorkbenchPage.h>

#include "QmitkStdMultiWidgetEditor.h"

QmitkFileOpenAction::QmitkFileOpenAction(cherry::IWorkbenchWindow::Pointer window)
: QAction(0)
{
  m_Window = window;
  this->setParent(static_cast<QWidget*>(m_Window->GetShell()->GetControl()));
  this->setText("&Open...");

  m_Window = window;

  this->connect(this, SIGNAL(triggered(bool)), this, SLOT(Run()));
}

void QmitkFileOpenAction::Run()
{
  /**
   * @brief stores the last path of last opened file
   */
  static QString m_LastPath; 

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
  
  mitk::DataStorageEditorInput::Pointer editorInput;
  mitk::DataStorage::Pointer dataStorage;
  QmitkStdMultiWidgetEditor::Pointer multiWidgetEditor;
  cherry::IEditorPart::Pointer editor = m_Window->GetActivePage()->GetActiveEditor();
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
    if ( fileName->right(5) == ".mitk" ) 
    {
      mitk::SceneIO::Pointer sceneIO = mitk::SceneIO::New();

      bool clearDataStorageFirst(false);
      mitk::ProgressBar::GetInstance()->AddStepsToDo(2);
      dataStorage = sceneIO->LoadScene( fileName->toLocal8Bit().constData(), dataStorage, clearDataStorageFirst );
      dsmodified = true;
      mitk::ProgressBar::GetInstance()->Progress(2);
    }
    else
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
  }
  
  if (multiWidgetEditor.IsNull())
  {
    cherry::IEditorPart::Pointer editor = m_Window->GetActivePage()->OpenEditor(editorInput, QmitkStdMultiWidgetEditor::EDITOR_ID);
    multiWidgetEditor = editor.Cast<QmitkStdMultiWidgetEditor>();
  }
  else
  {
    multiWidgetEditor->GetStdMultiWidget()->RequestUpdate();
  }

  if(dsmodified)
  {
    mitk::TimeSlicedGeometry::Pointer geometry = dataStorage->ComputeBoundingGeometry3D();

    //if ( geometry.IsNotNull() )
    //{
    //  // let's see if we have data with a limited live-span ...
    //  mitk::TimeBounds timebounds = geometry->GetTimeBounds();
    //  if ( timebounds[1] < mitk::ScalarTypeNumericTraits::max() )
    //  {
    //    mitk::ScalarType duration = timebounds[1]-timebounds[0];

    //    mitk::TimeSlicedGeometry::Pointer timegeometry =
    //      mitk::TimeSlicedGeometry::New();
    //    timegeometry->InitializeEvenlyTimed(
    //      geometry, (unsigned int) duration );
    //    timegeometry->SetTimeBounds( timebounds );

    //    timebounds[1] = timebounds[0] + 1.0;
    //    geometry->SetTimeBounds( timebounds );

    //    geometry = timegeometry;
    //  }
    //}
    mitk::RenderingManager::GetInstance()->InitializeViews(geometry);
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }

 
}
