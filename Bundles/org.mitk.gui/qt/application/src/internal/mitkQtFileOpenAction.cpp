#include "mitkQtFileOpenAction.h"

#include <QFileDialog>
#include <mitkDataTreeNodeFactory.h>

#include "../mitkDataStorageEditorInput.h"
#include "../mitkStdMultiWidgetEditor.h"

mitkQtFileOpenAction::mitkQtFileOpenAction(cherry::QtWorkbenchWindow::Pointer window)
 : QAction(0)
{
  m_Window = window;
  this->setParent(m_Window);
  this->setText("&Open...");
  
  m_Window = window;
  
  this->connect(this, SIGNAL(triggered(bool)), this, SLOT(Run()));
}

void mitkQtFileOpenAction::Run()
{
  QFileDialog dialog(m_Window);
  dialog.setFileMode(QFileDialog::ExistingFiles);
  dialog.setFilter("Images (*.pic *.pic.gz)");
  QStringList fileNames;
  if (dialog.exec())
    fileNames = dialog.selectedFiles();
  
  if (fileNames.empty()) return;
  
  mitkDataStorageEditorInput::Pointer editorInput = new mitkDataStorageEditorInput();
  
  mitk::DataStorage::Pointer dataStorage = editorInput->GetDataStorageReference()->GetDataStorage();
  
  
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
  
  m_Window->GetActivePage()->OpenEditor(editorInput, mitkStdMultiWidgetEditor::EDITOR_ID);
  
  
}

