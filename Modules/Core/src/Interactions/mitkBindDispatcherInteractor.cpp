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

#include "mitkBindDispatcherInteractor.h"
#include "mitkMessage.h"
#include <string.h>

// us
#include "usGetModuleContext.h"
#include "usModule.h"
#include "usModuleRegistry.h"

mitk::BindDispatcherInteractor::BindDispatcherInteractor( const std::string& rendererName ) :
    m_DataStorage(NULL)
{
  us::ModuleContext* context = us::ModuleRegistry::GetModule(1)->GetModuleContext();
  if (context == NULL)
  {
    MITK_ERROR<< "BindDispatcherInteractor() - Context could not be obtained.";
    return;
  }

  m_Dispatcher = Dispatcher::New(rendererName);
}

void mitk::BindDispatcherInteractor::SetDataStorage(mitk::DataStorage::Pointer dataStorage)
{
  // Set/Change Datastorage. This registers BDI to listen for events of DataStorage, to be informed when
  // a DataNode with a Interactor is added/modified/removed.
  if (dataStorage != m_DataStorage)
  {
    // clean up events from previous datastorage
    UnRegisterDataStorageEvents();
    m_DataStorage = dataStorage;
    RegisterDataStorageEvents();

    // Register existing interactors
    auto nodes = m_DataStorage->GetAll();
    for (auto node : *nodes) 
    {
      RegisterInteractor(node);
    }
  }
}

mitk::BindDispatcherInteractor::~BindDispatcherInteractor()
{
  if (m_DataStorage.IsNotNull())
  {
    UnRegisterDataStorageEvents();
  }
}

void mitk::BindDispatcherInteractor::RegisterInteractor(const mitk::DataNode* dataNode)
{
  if (m_Dispatcher.IsNotNull())
  {
    m_Dispatcher->AddDataInteractor(dataNode);
  }

}

void mitk::BindDispatcherInteractor::RegisterDataStorageEvents()
{
  if (m_DataStorage.IsNotNull())
  {
    m_DataStorage->AddNodeEvent.AddListener(
        MessageDelegate1<BindDispatcherInteractor, const DataNode*>(this, &BindDispatcherInteractor::RegisterInteractor));

    m_DataStorage->RemoveNodeEvent.AddListener(
        MessageDelegate1<BindDispatcherInteractor, const DataNode*>(this, &BindDispatcherInteractor::UnRegisterInteractor));

    m_DataStorage->InteractorChangedNodeEvent.AddListener(
        MessageDelegate1<BindDispatcherInteractor, const DataNode*>(this, &BindDispatcherInteractor::RegisterInteractor));
  }
}

void mitk::BindDispatcherInteractor::UnRegisterInteractor(const DataNode* dataNode)
{
  if (m_Dispatcher.IsNotNull())
  {
    m_Dispatcher->RemoveDataInteractor(dataNode);
  }
}

mitk::Dispatcher::Pointer mitk::BindDispatcherInteractor::GetDispatcher() const
{
  return m_Dispatcher;
}

void mitk::BindDispatcherInteractor::SetDispatcher(Dispatcher::Pointer dispatcher)
{
  m_Dispatcher = dispatcher;
}

void mitk::BindDispatcherInteractor::UnRegisterDataStorageEvents()
{
  if (m_DataStorage.IsNotNull())
  {
    m_DataStorage->AddNodeEvent.RemoveListener(
        MessageDelegate1<BindDispatcherInteractor, const DataNode*>(this, &BindDispatcherInteractor::RegisterInteractor));
    m_DataStorage->RemoveNodeEvent.RemoveListener(
        MessageDelegate1<BindDispatcherInteractor, const DataNode*>(this, &BindDispatcherInteractor::UnRegisterInteractor));
    m_DataStorage->InteractorChangedNodeEvent.RemoveListener(
        MessageDelegate1<BindDispatcherInteractor, const DataNode*>(this, &BindDispatcherInteractor::RegisterInteractor));
  }
}
