/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkPropertyDeserialization.h>
#include <mitkBaseProperty.h>

mitk::PropertyDeserialization::PropertyDeserialization()
{
}

mitk::PropertyDeserialization::~PropertyDeserialization()
{
}

mitk::BaseProperty::Pointer mitk::PropertyDeserialization::CreateInstance(const std::string& className)
{
  auto it = m_Map.find(className);

  if (it != m_Map.end())
    return static_cast<BaseProperty*>(it->second->CreateAnother().GetPointer());

  return nullptr;
}

void mitk::PropertyDeserialization::InternalRegisterProperty(const BaseProperty* property)
{
  m_Map[property->GetNameOfClass()] = property;
}
