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

#include "mitkDataStorageReference.h"

namespace mitk {

DataStorageReference::DataStorageReference(DataStorage::Pointer dataStorage, bool isDefault)
: m_Default(isDefault), m_DataStorage(dataStorage)
{

}

DataStorage::Pointer DataStorageReference::GetDataStorage() const
{
  return m_DataStorage;
}


bool DataStorageReference::IsDefault() const
{
  return m_Default;
}

QString DataStorageReference::GetLabel() const
{
  return m_Label;
}

void DataStorageReference::SetLabel(const QString& label)
{
  m_Label = label;
}

bool DataStorageReference::operator==(const berry::Object* o) const
{
  const DataStorageReference* other = dynamic_cast<const DataStorageReference*>(o);
  if (other == nullptr) return false;
  return (m_DataStorage == other->m_DataStorage);
}

}
