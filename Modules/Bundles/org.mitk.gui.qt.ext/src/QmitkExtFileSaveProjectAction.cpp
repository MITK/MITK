/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-10-07 16:14:59 +0200 (Mi, 07 Okt 2009) $
Version:   $Revision: 19343 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "QmitkExtFileSaveProjectAction.h"

#include <QFileDialog>
#include <QMessageBox>

#include "mitkSceneIO.h"
#include "mitkProgressBar.h"
#include <mitkNodePredicateNOT.h>
#include <mitkNodePredicateProperty.h>
#include <mitkProperties.h>

#include <mitkCoreObjectFactory.h>
#include <mitkDataStorageEditorInput.h>
#include <berryIEditorPart.h>
#include <berryIWorkbenchPage.h>

#include "QmitkStdMultiWidgetEditor.h"

QmitkExtFileSaveProjectAction::QmitkExtFileSaveProjectAction(berry::IWorkbenchWindow::Pointer window)
: QAction(0)
{
  m_Window = window;
  this->setParent(static_cast<QWidget*>(m_Window->GetShell()->GetControl()));
  this->setText("&Save Project...");
  this->setToolTip("Save content of Data Manager as a .mitk project file");
  m_Window = window;

  this->connect(this, SIGNAL(triggered(bool)), this, SLOT(Run()));
}

void QmitkExtFileSaveProjectAction::Run()
{
  try
  {
    QString fileName = QFileDialog::getSaveFileName(NULL, "Save MITK scene", QString::null, "MITK scene files (*.mitk)", NULL );

    if (fileName.isEmpty() ) 
      return;

    if ( fileName.right(5) != ".mitk" ) 
      fileName += ".mitk";

    mitk::SceneIO::Pointer sceneIO = mitk::SceneIO::New();

    mitk::DataStorageEditorInput::Pointer editorInput;
    mitk::DataStorage::Pointer storage;
    QmitkStdMultiWidgetEditor::Pointer multiWidgetEditor;
    berry::IEditorPart::Pointer editor = m_Window->GetActivePage()->GetActiveEditor();
    if (editor.Cast<QmitkStdMultiWidgetEditor>().IsNull())
    {
      editorInput = new mitk::DataStorageEditorInput();
      storage = editorInput->GetDataStorageReference()->GetDataStorage();
    }
    else
    {
      multiWidgetEditor = editor.Cast<QmitkStdMultiWidgetEditor>();
      storage = multiWidgetEditor->GetEditorInput().Cast<mitk::DataStorageEditorInput>()->GetDataStorageReference()->GetDataStorage();
    }

    mitk::ProgressBar::GetInstance()->AddStepsToDo(2);

    /* Build list of nodes that should be saved */
    
    mitk::NodePredicateNOT::Pointer isNotHelperObject = mitk::NodePredicateNOT::New(mitk::NodePredicateProperty::New("helper object", mitk::BoolProperty::New(true)));

    mitk::DataStorage::SetOfObjects::ConstPointer nodesToBeSaved = storage->GetSubset(isNotHelperObject);
    if ( !sceneIO->SaveScene( nodesToBeSaved, storage, fileName.toLocal8Bit().constData() ) )
    {
      QMessageBox::information(NULL,
        "Scene saving",
        "Scene could not be written completely. Please check the log.",
        QMessageBox::Ok);

    }
    mitk::ProgressBar::GetInstance()->Progress(2);

    mitk::SceneIO::FailedBaseDataListType::ConstPointer failedNodes = sceneIO->GetFailedNodes();
    if (failedNodes.IsNotNull())
    {
      std::stringstream ss;
      ss << "The following nodes could not be serialized:" << std::endl;
      for ( mitk::SceneIO::FailedBaseDataListType::const_iterator iter = failedNodes->begin();
        iter != failedNodes->end();
        ++iter )
      {
        ss << " - ";
        if ( mitk::BaseData* data =(*iter)->GetData() )
        {
          ss << data->GetNameOfClass();
        }
        else
        {
          ss << "(NULL)";
        }

        ss << " contained in node '" << (*iter)->GetName() << "'" << std::endl;
      }

      MITK_WARN << ss.str();
    }

    mitk::PropertyList::ConstPointer failedProperties = sceneIO->GetFailedProperties();
    if (failedProperties.IsNotNull())
    {
      std::stringstream ss;
      ss << "The following properties could not be serialized:" << std::endl;
      const mitk::PropertyList::PropertyMap* propmap = failedProperties->GetMap();
      for ( mitk::PropertyList::PropertyMap::const_iterator iter = propmap->begin();
        iter != propmap->end();
        ++iter )
      {
        ss << " - " << iter->second.first->GetNameOfClass() << " associated to key '" << iter->first << "'" << std::endl;
      }

      MITK_WARN << ss.str();
    }
  }
  catch (std::exception& e)
  {
    MITK_ERROR << "Exception caught during scene saving: " << e.what();
  }
}
