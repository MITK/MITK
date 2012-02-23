/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkDataStorageService.h"

#include "mitkDataStorageReference.h"
#include "mitkStandaloneDataStorage.h"

namespace mitk {

DataStorageService::DataStorageService()
{
  m_DefaultDataStorageRef = this->CreateDataStorage("Default DataStorage");
  m_ActiveDataStorageRef = m_DefaultDataStorageRef;
}

bool
DataStorageService::IsA(const std::type_info& type) const
{
  std::string name(GetType().name());
  return name == type.name() || Service::IsA(type);
}

const std::type_info&
DataStorageService::GetType() const
{
  return typeid(IDataStorageService);
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
    m_ActiveDataStorageRef = 0;
  }

  return m_DataStorageReferences.erase(dataStorageRef);
}

}
