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
#include <mitkDataTreeNodeFactory.h>

#include <mitkDataStorageEditorInput.h>
#include <cherryIEditorPart.h>
#include <cherryIWorkbenchPage.h>

#include "../QmitkStdMultiWidgetEditor.h"

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
  QFileDialog dialog(static_cast<QWidget*>(m_Window->GetShell()->GetControl()));
  dialog.setFileMode(QFileDialog::ExistingFiles);
  QStringList filters;
  filters << "Images (*.pic *.pic.gz *.vti *.dcm)" 
    << "Surfaces (*.stl *.vtk *.vtp)"
    << "MITK Pointset (*.mps)"
    << "All Files (*.*)";
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

  mitk::DataTreeNode::Pointer node;
  for (QStringList::Iterator fileName = fileNames.begin();
    fileName != fileNames.end(); ++fileName)
  {
    mitk::DataTreeNodeFactory::Pointer nodeReader = mitk::DataTreeNodeFactory::New();
    try
    {
      nodeReader->SetFileName(fileName->toStdString());
      nodeReader->Update();
      node = nodeReader->GetOutput();
      std::cout << "node name on line 78: " << node->GetName() << std::endl;
      dataStorage->Add(node);
      std::cout << "node name on line 80: " << node->GetName() << std::endl;
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
    multiWidgetEditor->GetStdMultiWidget()->RequestUpdate();
  }        

  if(node.IsNotNull() && node->GetData() != 0)
  {
    mitk::RenderingManager::GetInstance()->InitializeViews(dataStorage->ComputeBoundingGeometry3D(dataStorage->GetAll()));
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
}
