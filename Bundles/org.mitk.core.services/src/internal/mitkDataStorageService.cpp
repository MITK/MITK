#include "mitkDataStorageService.h"

#include "mitkDataStorageReference.h"

#include <mitkDataTree.h>

namespace mitk {

DataStorageService::DataStorageService()
{
  m_DefaultDataStorageRef = this->CreateDefaultDataStorage();
  m_ActiveDataStorageRef = m_DefaultDataStorageRef;
}

bool
DataStorageService::IsA(const std::type_info& type)
{
  std::string name(GetType().name());
  return name == type.name() || Service::IsA(type);
}

const std::type_info&
DataStorageService::GetType() const
{
  return typeid(IDataStorageService);
}

IDataStorageReference::Pointer DataStorageService::CreateDataStorage(const std::string& label)
{
  DataTree::Pointer dataTree = DataTree::New();
  DataStorage::Pointer dataStorage = DataStorage::CreateInstance(dataTree);
  DataStorageReference::Pointer ref(new DataStorageReference(dataStorage, dataTree));
  ref->SetLabel(label);
  m_DataStorageReferences.push_back(ref);

  return ref;
}


std::vector<IDataStorageReference::Pointer> DataStorageService::GetDataStorageReferences() const
{
  return m_DataStorageReferences;
}

IDataStorageReference::Pointer DataStorageService::GetDefaultDataStorage() const
{
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

IDataStorageReference::Pointer DataStorageService::CreateDefaultDataStorage()
{
  DataTree::Pointer dataTree = DataTree::New();
  DataStorage::Pointer dataStorage = DataStorage::CreateInstance(dataTree);
  DataStorageReference::Pointer ref(new DataStorageReference(dataStorage, dataTree, true));
  ref->SetLabel("Default DataStorage");
  m_DataStorageReferences.push_back(ref);

  return ref;
}

}
