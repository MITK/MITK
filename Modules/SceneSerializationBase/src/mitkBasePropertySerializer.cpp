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

#include "mitkBasePropertySerializer.h"

mitk::BasePropertySerializer::BasePropertySerializer()
{
}

mitk::BasePropertySerializer::~BasePropertySerializer()
{
}

TiXmlElement* mitk::BasePropertySerializer::Serialize()
{
  MITK_INFO << this->GetNameOfClass()
           << " is asked to serialize an object " << (const void*) this->m_Property;

  return NULL;
}

mitk::BaseProperty::Pointer mitk::BasePropertySerializer::Deserialize(TiXmlElement*)
{
  MITK_ERROR << this->GetNameOfClass() << " is asked to deserialize an object but has no implementation. This is bad.";
  return NULL;
}
