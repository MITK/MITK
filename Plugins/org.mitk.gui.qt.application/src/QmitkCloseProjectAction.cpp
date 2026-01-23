/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkCloseProjectAction.h"

#include <mitkCoreServices.h>
#include <mitkDataStorageEditorInput.h>
#include <mitkIDataStorageService.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateProperty.h>
#include <mitkProperties.h>
#include <mitkUndoController.h>

#include <berryIEditorReference.h>
#include <berryIWorkbenchPage.h>
#include <berryIWorkbenchWindow.h>

#include <QMessageBox>

QmitkCloseProjectAction::QmitkCloseProjectAction(berry::IWorkbenchWindow::Pointer window)
  : QAction(nullptr)
  , m_Window(nullptr)
{
  this->init(window.GetPointer());
}

QmitkCloseProjectAction::QmitkCloseProjectAction(const QIcon & icon, berry::IWorkbenchWindow::Pointer window)
  : QAction(nullptr)
  , m_Window(nullptr)
{
  this->setIcon(icon);
  this->init(window.GetPointer());
}

QmitkCloseProjectAction::QmitkCloseProjectAction(berry::IWorkbenchWindow* window)
  : QAction(nullptr)
  , m_Window(nullptr)
{
  this->init(window);
}

QmitkCloseProjectAction::QmitkCloseProjectAction(const QIcon& icon, berry::IWorkbenchWindow* window)
  : QAction(nullptr)
  , m_Window(nullptr)
{
  this->setIcon(icon);
  this->init(window);
}

void QmitkCloseProjectAction::init(berry::IWorkbenchWindow* window)
{
  m_Window = window;
  this->setText("&Close Project...");
  this->setToolTip("Close Project will remove all data objects from the application. This will free up the memory that is used by the data.");
  this->connect(this, SIGNAL(triggered(bool)), this, SLOT(Run()));
}

void QmitkCloseProjectAction::Run()
{
  try
  {
    mitk::CoreServicePointer<mitk::IDataStorageService> dsService(mitk::CoreServices::GetDataStorageService());

    if (!dsService)
    {
      MITK_WARN << "IDataStorageService service not available. Unable to close project.";
      return;
    }

    mitk::DataStorageReference storageInfo = dsService->GetActiveDataStorageReference();
    mitk::DataStorage::Pointer dataStorage = storageInfo.GetStorage();

    if (dataStorage.IsNull())
    {
      MITK_WARN << "No data storage available. Cannot close project.";
      return;
    }

    // Check if we got the default datastorage and if there is anything else than helper objects in the storage
    if (storageInfo.IsDefault() &&
        dataStorage->GetSubset(mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("helper object", mitk::BoolProperty::New(true))))->empty())
    {
      return;
    }

    /* Ask, if the user is sure about that */
    QString msg = "Are you sure that you want to close the current project (%1)?\nThis will remove all data objects.";
    if (QMessageBox::question(nullptr, "Remove all data?", msg.arg(QString::fromStdString(storageInfo.GetLabel())),
                              QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
    {
      return;
    }

    /* Remove everything */
    {
      mitk::DataStorage::SetOfObjects::ConstPointer nodesToRemove = dataStorage->GetAll();
      dataStorage->Remove(nodesToRemove);
    }

    // Explicitly trigger undo model again. It is also done in the data storage itself, but
    // as long as the nodesToRemove variable exists, it has no effect, as the smart pointer keeps
    // the removed nodes alive due to the list of smart pointer selections.
    if (auto undoModel = mitk::UndoController::GetCurrentUndoModel(); nullptr != undoModel)
    {
      undoModel->RemoveInvalidOperations();
    }

    // Remove the datastorage from the data storage service (unless it's the default)
    if (!storageInfo.IsDefault())
    {
      dsService->RemoveDataStorage(storageInfo.GetLabel());
    }

    // Close all editors with this data storage as input
    mitk::DataStorageEditorInput::Pointer dsInput(new mitk::DataStorageEditorInput(storageInfo));
    QList<berry::IEditorReference::Pointer> dsEditors =
        m_Window->GetActivePage()->FindEditors(dsInput, QString(), berry::IWorkbenchPage::MATCH_INPUT);

    if (!dsEditors.empty())
    {
      QList<berry::IEditorReference::Pointer> editorsToClose = dsEditors;
      m_Window->GetActivePage()->CloseEditors(editorsToClose, false);
    }
  }
  catch (std::exception& e)
  {
    MITK_ERROR << "Exception caught during closing project: " << e.what();
    QMessageBox::warning(nullptr, "Error", QString("An error occurred during Close Project: %1").arg(e.what()));
  }
}
