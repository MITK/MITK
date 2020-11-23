/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkEnumerationPropertySerializer.h"
#include <tinyxml2.h>

namespace mitk
{
  tinyxml2::XMLElement *EnumerationPropertySerializer::Serialize(tinyxml2::XMLDocument &doc)
  {
    if (const auto *prop = dynamic_cast<const EnumerationProperty *>(m_Property.GetPointer()))
    {
      auto element = doc.NewElement("enum");
      element->SetAttribute("value", prop->GetValueAsString().c_str());
      return element;
    }
    else
      return nullptr;
  }

  EnumerationPropertySerializer::EnumerationPropertySerializer() {}
  EnumerationPropertySerializer::~EnumerationPropertySerializer() {}
} // namespace

// important to put this into the GLOBAL namespace (because it starts with 'namespace mitk')
MITK_REGISTER_SERIALIZER(EnumerationPropertySerializer);
