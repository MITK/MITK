/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkDataStorageService.h"

#include "mitkDataStorageReference.h"
#include "mitkStandaloneDataStorage.h"

namespace mitk {

DataStorageService::DataStorageService()
{
  m_DefaultDataStorageRef = this->CreateDataStorage("Default DataStorage");
  m_ActiveDataStorageRef = m_DefaultDataStorageRef;
}

IDataStorageReference::Pointer DataStorageService::CreateDataStorage(const QString& label)
{

  StandaloneDataStorage::Pointer dataStorage = mitk::StandaloneDataStorage::New();
  DataStorageReference::Pointer ref(new DataStorageReference(dataStorage.GetPointer()));
  ref->SetLabel(label);
  m_DataStorageReferences.insert(ref);

  return ref;
}


std::vector<IDataStorageReference::Pointer> DataStorageService::GetDataStorageReferences() const
{
  std::vector<IDataStorageReference::Pointer> res;
  res.reserve(m_DataStorageReferences.size()+1);
  res.push_back(m_DefaultDataStorageRef);
  res.insert(res.end(), m_DataStorageReferences.begin(), m_DataStorageReferences.end());
  return res;
}

IDataStorageReference::Pointer DataStorageService::GetDefaultDataStorage() const
{
  return m_DefaultDataStorageRef;
}

IDataStorageReference::Pointer DataStorageService::GetDataStorage() const
{
  if (m_ActiveDataStorageRef.IsNotNull()) return m_ActiveDataStorageRef;
  return m_DefaultDataStorageRef;
}

IDataStorageReference::Pointer DataStorageService::GetActiveDataStorage() const
{
  return m_ActiveDataStorageRef;
}

void DataStorageService::SetActiveDataStorage(IDataStorageReference::Pointer dataStorageRef)
{
  if (dataStorageRef.IsNull()) m_ActiveDataStorageRef = m_DefaultDataStorageRef;
  else m_ActiveDataStorageRef = dataStorageRef;
}

void DataStorageService::AddDataStorageReference(IDataStorageReference::Pointer dataStorageRef)
{
  if (dataStorageRef.IsNull() || dataStorageRef->IsDefault()) return;

  m_DataStorageReferences.insert(dataStorageRef);
}

bool DataStorageService::RemoveDataStorageReference(IDataStorageReference::Pointer dataStorageRef)
{
  if (m_ActiveDataStorageRef == dataStorageRef)
  {
    m_ActiveDataStorageRef = nullptr;
  }

  return m_DataStorageReferences.erase(dataStorageRef);
}

}
