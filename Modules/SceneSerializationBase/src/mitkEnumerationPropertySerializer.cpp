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

#include "mitkEnumerationPropertySerializer.h"

namespace mitk
{

TiXmlElement* EnumerationPropertySerializer::Serialize()
{
  if (const EnumerationProperty* prop = dynamic_cast<const EnumerationProperty*>(m_Property.GetPointer()))
  {
    auto  element = new TiXmlElement("enum");
    element->SetAttribute("value", prop->GetValueAsString());
    return element;
  }
  else return nullptr;
}

EnumerationPropertySerializer::EnumerationPropertySerializer()
{}

EnumerationPropertySerializer::~EnumerationPropertySerializer()
{}

} // namespace

// important to put this into the GLOBAL namespace (because it starts with 'namespace mitk')
MITK_REGISTER_SERIALIZER(EnumerationPropertySerializer);

