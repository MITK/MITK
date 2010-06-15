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

#include "QmitkCloseProjectAction.h"

#include <mitkCoreObjectFactory.h>
#include <mitkDataStorageEditorInput.h>
#include <berryIEditorPart.h>
#include <berryIWorkbenchPage.h>
#include "QmitkStdMultiWidgetEditor.h"

#include <QMessageBox>

QmitkCloseProjectAction::QmitkCloseProjectAction(berry::IWorkbenchWindow::Pointer window)
: QAction(0)
{
  this->init(window);
}

QmitkCloseProjectAction::QmitkCloseProjectAction(const QIcon & icon, berry::IWorkbenchWindow::Pointer window)
: QAction(0)
{
  this->setIcon(icon);
  this->init(window);
}

void QmitkCloseProjectAction::init(berry::IWorkbenchWindow::Pointer window)
{
  m_Window = window;
  this->setParent(static_cast<QWidget*>(m_Window->GetShell()->GetControl()));
  this->setText("&Close Project...");
  this->setToolTip("Close Project will remove all data objects from the application. This will free up the memory that is used by the data.");
  m_Window = window;
  this->connect(this, SIGNAL(triggered(bool)), this, SLOT(Run()));
}

void QmitkCloseProjectAction::Run()
{
  /* Ask, if the user is sure about that */
  if (QMessageBox::question(NULL, "Remove all data?", "Are you sure that you want to close the current project? This will remove all data objects?", QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
    return;

  try
  {
    /* Get the DataStorage and the MultiWidget */
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
    /* Remove everything */
    mitk::DataStorage::SetOfObjects::ConstPointer nodesToRemove = storage->GetAll();
    storage->Remove(nodesToRemove);
    /* Re-Add 2D renderer planes */
    if (multiWidgetEditor.IsNotNull())
      multiWidgetEditor->GetStdMultiWidget()->AddPlanesToDataStorage();
  }
  catch (std::exception& e)
  {
    MITK_ERROR << "Exception caught during scene saving: " << e.what();
    QMessageBox::warning(NULL, "Error", QString("An error occurred during Close Project: %1").arg(e.what()));
  }
}
