/*=========================================================================
 
Program:   openCherry Platform
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
#include <mitkDataTreeNodeFactory.h>

#include <mitkDataStorageEditorInput.h>
#include <cherryIEditorPart.h>

#include "../QmitkStdMultiWidgetEditor.h"

QmitkFileOpenAction::QmitkFileOpenAction(cherry::QtWorkbenchWindow::Pointer window)
 : QAction(0)
{
  m_Window = window;
  this->setParent(m_Window);
  this->setText("&Open...");
  
  m_Window = window;
  
  this->connect(this, SIGNAL(triggered(bool)), this, SLOT(Run()));
}

void QmitkFileOpenAction::Run()
{
  QFileDialog dialog(m_Window);
  dialog.setFileMode(QFileDialog::ExistingFiles);
  QStringList filters;
  filters << "Images (*.pic *.pic.gz)" << "All Files (*.*)";
  dialog.setFilters(filters);
  QStringList fileNames;
  if (dialog.exec())
    fileNames = dialog.selectedFiles();
  
  if (fileNames.empty()) return;

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
  
  for (QStringList::Iterator fileName = fileNames.begin();
       fileName != fileNames.end(); ++fileName)
  {
    mitk::DataTreeNodeFactory::Pointer nodeReader = mitk::DataTreeNodeFactory::New();
    try
    {
      nodeReader->SetFileName(fileName->toStdString());
      nodeReader->Update();
      dataStorage->Add(nodeReader->GetOutput());
    }
    catch(...)
    {
      
    }
  }
  
  if (multiWidgetEditor.IsNull())
  {
    cherry::IEditorPart::Pointer editor = m_Window->GetActivePage()->OpenEditor(editorInput, QmitkStdMultiWidgetEditor::EDITOR_ID);
    multiWidgetEditor = editor.Cast<QmitkStdMultiWidgetEditor>();
  }
  else
  {
    multiWidgetEditor->GetStdMultiWidget()->ForceImmediateUpdate();
  }
}

