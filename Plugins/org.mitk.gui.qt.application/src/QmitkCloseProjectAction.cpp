/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "QmitkCloseProjectAction.h"
#include "internal/org_mitk_gui_qt_application_Activator.h"

#include <mitkIDataStorageService.h>
#include <mitkDataStorageEditorInput.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateProperty.h>
#include <mitkProperties.h>

#include <berryIEditorReference.h>
#include <berryIWorkbenchPage.h>

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


  try
  {
    ctkPluginContext* context = mitk::PluginActivator::GetContext();
    mitk::IDataStorageService* dss = 0;
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
    if (QMessageBox::question(NULL, "Remove all data?", msg.arg(dataStorageRef->GetLabel()),
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
    std::vector<berry::IEditorReference::Pointer> dsEditors =
        m_Window->GetActivePage()->FindEditors(dsInput, std::string(), berry::IWorkbenchPage::MATCH_INPUT);

    if (!dsEditors.empty())
    {
      std::list<berry::IEditorReference::Pointer> editorsToClose;
      editorsToClose.assign(dsEditors.begin(), dsEditors.end());
      m_Window->GetActivePage()->CloseEditors(editorsToClose, false);
    }
  }
  catch (std::exception& e)
  {
    MITK_ERROR << "Exception caught during closing project: " << e.what();
    QMessageBox::warning(NULL, "Error", QString("An error occurred during Close Project: %1").arg(e.what()));
  }
}
