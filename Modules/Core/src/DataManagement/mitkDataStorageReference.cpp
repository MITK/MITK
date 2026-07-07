/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkDataStorageReference.h>

namespace mitk
{

DataStorageReference::DataStorageReference()
  : m_IsDefault(false)
{
}

DataStorageReference::DataStorageReference(const std::string& label, DataStorage::Pointer storage, bool isDefault)
  : m_Label(label),
    m_Storage(storage),
    m_IsDefault(isDefault)
{
}

std::string DataStorageReference::GetLabel() const
{
  return m_Label;
}

void DataStorageReference::SetLabel(const std::string& label)
{
  m_Label = label;
}

DataStorage::Pointer DataStorageReference::GetStorage() const
{
  return m_Storage;
}

void DataStorageReference::SetStorage(DataStorage::Pointer storage)
{
  m_Storage = storage;
}

bool DataStorageReference::IsDefault() const
{
  return m_IsDefault;
}

bool DataStorageReference::IsValid() const
{
  return m_Storage.IsNotNull();
}

bool DataStorageReference::operator==(const DataStorageReference& other) const
{
  return m_Storage.GetPointer() == other.m_Storage.GetPointer();
}

bool DataStorageReference::operator!=(const DataStorageReference& other) const
{
  return !(*this == other);
}

} // namespace mitk
