/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkDataStorageEditorInput.h"

#include <berryPlatform.h>
#include <berryIAdapterManager.h>
#include <berryIMemento.h>

#include <mitkIDataStorageService.h>

#include "internal/org_mitk_gui_common_Activator.h"
#include "internal/mitkDataStorageEditorInputFactory.h"

#include <QIcon>

namespace mitk
{

DataStorageEditorInput::DataStorageEditorInput()
{
}

DataStorageEditorInput::DataStorageEditorInput(IDataStorageReference::Pointer ref)
{
  m_DataStorageRef = ref;
}

bool DataStorageEditorInput::Exists() const
{
  return true;
}

QString DataStorageEditorInput::GetName() const
{
  return "DataStorage Scene";
}

QString DataStorageEditorInput::GetToolTipText() const
{
  return "";
}

QIcon DataStorageEditorInput::GetIcon() const
{
  return QIcon();
}

const berry::IPersistableElement*DataStorageEditorInput::GetPersistable() const
{
  //return this;
  return nullptr;
}

berry::Object* DataStorageEditorInput::GetAdapter(const QString& adapterType) const
{
  berry::IAdapterManager* adapterManager = berry::Platform::GetAdapterManager();
  if (adapterManager)
  {
    return adapterManager->GetAdapter(this, adapterType);
  }
  return nullptr;
}

//QString DataStorageEditorInput::GetFactoryId() const
//{
//  return DataStorageEditorInputFactory::GetFactoryId();
//}

//void DataStorageEditorInput::SaveState(const berry::IMemento::Pointer& memento) const
//{
//  return DataStorageEditorInputFactory::SaveState(memento, this);
//}

bool DataStorageEditorInput::operator==(const berry::Object* o) const
{
  if (const DataStorageEditorInput* input = dynamic_cast<const DataStorageEditorInput*>(o))
    return this->m_DataStorageRef == input->m_DataStorageRef;

  return false;
}

IDataStorageReference::Pointer
DataStorageEditorInput::GetDataStorageReference()
{
  if (m_DataStorageRef.IsNull())
  {
    ctkPluginContext* context = PluginActivator::GetContext();
    ctkServiceReference serviceRef = context->getServiceReference<IDataStorageService>();
    if (!serviceRef) return IDataStorageReference::Pointer(nullptr);
    IDataStorageService* dataService = context->getService<IDataStorageService>(serviceRef);
    if (!dataService) return IDataStorageReference::Pointer(nullptr);
    m_DataStorageRef = dataService->GetDefaultDataStorage();
  }

  return m_DataStorageRef;
}

}
