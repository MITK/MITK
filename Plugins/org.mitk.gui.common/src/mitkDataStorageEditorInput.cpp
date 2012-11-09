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

#include "mitkDataStorageEditorInput.h"

#include <berryPlatform.h>
#include <mitkIDataStorageService.h>

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

std::string DataStorageEditorInput::GetName() const
{
  return "DataStorage Scene";
}

std::string DataStorageEditorInput::GetToolTipText() const
{
  return "";
}

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
    berry::ServiceRegistry& serviceRegistry = berry::Platform::GetServiceRegistry();
    IDataStorageService::Pointer dataService = serviceRegistry.GetServiceById<IDataStorageService>(IDataStorageService::ID);
    if (!dataService) return IDataStorageReference::Pointer(0);
    m_DataStorageRef = dataService->GetDefaultDataStorage();
  }

  return m_DataStorageRef;
}

}
