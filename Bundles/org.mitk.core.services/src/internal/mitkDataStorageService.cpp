#include "mitkDataStorageService.h"

#include "mitkDataStorageReference.h"

#include <mitkDataTree.h>

namespace mitk {

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
  DataStorageReference::Pointer ref = new DataStorageReference(dataStorage, dataTree);
  ref->SetLabel(label);
  m_DataStorageReferences.push_back(ref);
  
  return ref;
}
  
  
std::vector<IDataStorageReference::Pointer> DataStorageService::GetDataStorageReferences()
{
  return m_DataStorageReferences;
}

}
