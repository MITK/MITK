/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkCloseProjectAction.h"
#include "internal/org_mitk_gui_qt_application_Activator.h"

#include <mitkIDataStorageService.h>
#include <mitkDataStorageEditorInput.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateProperty.h>
#include <mitkProperties.h>

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
    ctkPluginContext* context = mitk::PluginActivator::GetContext();
    mitk::IDataStorageService* dss = nullptr;
    ctkServiceReference dsRef = context->getServiceReference<mitk::IDataStorageService>();
    if (dsRef)
    {
      dss = context->getService<mitk::IDataStorageService>(dsRef);
    }

    if (!dss)
    {
      MITK_WARN << "IDataStorageService service not available. Unable to close project.";
      context->ungetService(dsRef);
      return;
    }

    mitk::IDataStorageReference::Pointer dataStorageRef = dss->GetActiveDataStorage();
    if (dataStorageRef.IsNull())
    {
      // No active data storage set (i.e. not editor with a DataStorageEditorInput is active).
      dataStorageRef = dss->GetDefaultDataStorage();
    }

    mitk::DataStorage::Pointer dataStorage = dataStorageRef->GetDataStorage();
    if (dataStorage.IsNull())
    {
      MITK_WARN << "No data storage available. Cannot close project.";
      return;
    }

    //check if we got the default datastorage and if there is anything else then helper object in the storage
    if(dataStorageRef->IsDefault() &&
       dataStorage->GetSubset(mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("helper object", mitk::BoolProperty::New(true))))->empty())
    {
      return;
    }

    /* Ask, if the user is sure about that */
    QString msg = "Are you sure that you want to close the current project (%1)?\nThis will remove all data objects.";
    if (QMessageBox::question(nullptr, "Remove all data?", msg.arg(dataStorageRef->GetLabel()),
                              QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
    {
      return;
    }

    /* Remove everything */
    mitk::DataStorage::SetOfObjects::ConstPointer nodesToRemove = dataStorage->GetAll();
    dataStorage->Remove(nodesToRemove);

    // Remove the datastorage from the data storage service
    dss->RemoveDataStorageReference(dataStorageRef);

    // Close all editors with this data storage as input
    mitk::DataStorageEditorInput::Pointer dsInput(new mitk::DataStorageEditorInput(dataStorageRef));
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
