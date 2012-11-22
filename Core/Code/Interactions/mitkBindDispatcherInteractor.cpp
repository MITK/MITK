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

mitk::BindDispatcherInteractor::BindDispatcherInteractor()
{
  m_DataStorage = NULL;
}

void mitk::BindDispatcherInteractor::SetDataStorage(mitk::DataStorage::Pointer dataStorage)
{
  // Set/Change Datastorage. This registers BDI to listen for events of DataStorage, to be informed when
  // a DataNode with a Interactor is added/modified/removed.

  // clean up events from previous datastorage
  if (m_DataStorage.IsNotNull())
  {
    UnRegisterDataStorageEvents();
  }
  m_DataStorage = dataStorage;
  if (m_DataStorage.IsNotNull())
  {
    RegisterDataStorageEvents();
  }
}

void mitk::BindDispatcherInteractor::SetDispatcher(mitk::Dispatcher::Pointer dispatcher)
{
  m_Dispatcher = dispatcher;
}

mitk::BindDispatcherInteractor::~BindDispatcherInteractor()
{
  // TODO Auto-generated destructor stub
}

void mitk::BindDispatcherInteractor::RegisterInteractor(mitk::DataNode::Pointer interactor)
{

}

void mitk::BindDispatcherInteractor::RegisterDataStorageEvents()
{
  m_DataStorage->AddNodeEvent.AddListener(
        mitk::MessageDelegate1<BindDispatcherInteractor, const mitk::DataNode*>(this, &BindDispatcherInteractor::RegisterInteractor));
  m_DataStorage->RemoveNodeEvent.AddListener(
      mitk::MessageDelegate1<BindDispatcherInteractor, const mitk::DataNode*>(this, &BindDispatcherInteractor::UnRegisterInteractor));
  m_DataStorage->ChangedNodeEvent.AddListener(
      mitk::MessageDelegate1<BindDispatcherInteractor, const mitk::DataNode*>(this, &BindDispatcherInteractor::RegisterInteractor));
}

void mitk::BindDispatcherInteractor::UnRegisterInteractor(mitk::DataNode::Pointer dataNode)
{
}

void mitk::BindDispatcherInteractor::UnRegisterDataStorageEvents()
{
  m_DataStorage->AddNodeEvent.RemoveListener(
          mitk::MessageDelegate1<BindDispatcherInteractor, const mitk::DataNode*>(this, &BindDispatcherInteractor::RegisterInteractor));
  m_DataStorage->RemoveNodeEvent.RemoveListener(
      mitk::MessageDelegate1<BindDispatcherInteractor, const mitk::DataNode*>(this, &BindDispatcherInteractor::UnRegisterInteractor));
  m_DataStorage->ChangedNodeEvent.RemoveListener(
      mitk::MessageDelegate1<BindDispatcherInteractor, const mitk::DataNode*>(this, &BindDispatcherInteractor::RegisterInteractor));
}
