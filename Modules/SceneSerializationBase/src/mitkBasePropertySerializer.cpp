/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkBasePropertySerializer.h"

mitk::BasePropertySerializer::BasePropertySerializer()
{
}

mitk::BasePropertySerializer::~BasePropertySerializer()
{
}

TiXmlElement *mitk::BasePropertySerializer::Serialize()
{
  MITK_INFO << this->GetNameOfClass() << " is asked to serialize an object " << (const void *)this->m_Property;

  return nullptr;
}

mitk::BaseProperty::Pointer mitk::BasePropertySerializer::Deserialize(TiXmlElement *)
{
  MITK_ERROR << this->GetNameOfClass() << " is asked to deserialize an object but has no implementation. This is bad.";
  return nullptr;
}
