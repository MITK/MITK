#include "mitkDataStorageReference.h"

namespace mitk {

DataStorageReference::DataStorageReference(DataStorage::Pointer dataStorage,
    DataTree::Pointer dataTree)
: m_DataStorage(dataStorage), m_DataTree(dataTree)
{
  
}

DataStorage::Pointer DataStorageReference::GetDataStorage()
{
  return m_DataStorage;
}

DataTree::Pointer DataStorageReference::GetDataTree()
{
  return m_DataTree;
}
  
std::string DataStorageReference::GetLabel()
{
  return m_Label;
}

void DataStorageReference::SetLabel(const std::string& label)
{
  m_Label = label;
}
  
bool DataStorageReference::operator==(const IDataStorageReference* o)
{
  if (dynamic_cast<const DataStorageReference*>(o) == 0) return false;
  return (m_DataStorage == dynamic_cast<const DataStorageReference*>(o)->m_DataStorage);
}

}
