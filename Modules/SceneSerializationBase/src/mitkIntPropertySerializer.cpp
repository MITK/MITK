/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkIntPropertySerializer_h_included
#define mitkIntPropertySerializer_h_included

#include "mitkBasePropertySerializer.h"
#include "mitkProperties.h"
#include <tinyxml2.h>

namespace mitk
{
  class IntPropertySerializer : public BasePropertySerializer
  {
  public:
    mitkClassMacro(IntPropertySerializer, BasePropertySerializer)
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    tinyxml2::XMLElement *Serialize(tinyxml2::XMLDocument &doc) override
    {
      if (const IntProperty *prop = dynamic_cast<const IntProperty *>(m_Property.GetPointer()))
      {
        auto *element = doc.NewElement("int");
        element->SetAttribute("value", prop->GetValue());
        return element;
      }
      else
        return nullptr;
    }

    BaseProperty::Pointer Deserialize(const tinyxml2::XMLElement *element) override
    {
      if (!element)
        return nullptr;

      int integer;
      if (element->QueryIntAttribute("value", &integer) == tinyxml2::XML_SUCCESS)
      {
        return IntProperty::New(integer).GetPointer();
      }
      else
      {
        return nullptr;
      }
    }

  protected:
    IntPropertySerializer() {}
    ~IntPropertySerializer() override {}
  };

} // namespace

// important to put this into the GLOBAL namespace (because it starts with 'namespace mitk')
MITK_REGISTER_SERIALIZER(IntPropertySerializer);

#endif
