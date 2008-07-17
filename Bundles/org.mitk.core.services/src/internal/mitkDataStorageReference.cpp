#include "mitkDataStorageReference.h"

namespace mitk {

DataStorageReference::DataStorageReference(DataStorage::Pointer dataStorage,
    DataTree::Pointer dataTree, bool isDefault)
: m_Default(isDefault), m_DataStorage(dataStorage), m_DataTree(dataTree)
{
  
}

DataStorage::Pointer DataStorageReference::GetDataStorage() const
{
  return m_DataStorage;
}

DataTree::Pointer DataStorageReference::GetDataTree() const
{
  return m_DataTree;
}

bool DataStorageReference::IsDefault() const
{
  return m_Default;
}
  
std::string DataStorageReference::GetLabel() const
{
  return m_Label;
}

void DataStorageReference::SetLabel(const std::string& label)
{
  m_Label = label;
}
  
bool DataStorageReference::operator==(const IDataStorageReference* o) const
{
  if (dynamic_cast<const DataStorageReference*>(o) == 0) return false;
  return (m_DataStorage == dynamic_cast<const DataStorageReference*>(o)->m_DataStorage);
}

}
