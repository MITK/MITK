/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkStringPropertySerializer_h_included
#define mitkStringPropertySerializer_h_included

#include "mitkBasePropertySerializer.h"
#include "mitkStringProperty.h"
#include <tinyxml2.h>

namespace mitk
{
  class StringPropertySerializer : public BasePropertySerializer
  {
  public:
    mitkClassMacro(StringPropertySerializer, BasePropertySerializer)
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    tinyxml2::XMLElement *Serialize(tinyxml2::XMLDocument &doc) override
    {
      if (const StringProperty *prop = dynamic_cast<const StringProperty *>(m_Property.GetPointer()))
      {
        auto *element = doc.NewElement("string");
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
      const char *s(element->Attribute("value"));
      return StringProperty::New(std::string(s ? s : "")).GetPointer();
    }

  protected:
    StringPropertySerializer() {}
    ~StringPropertySerializer() override {}
  };

} // namespace

// important to put this into the GLOBAL namespace (because it starts with 'namespace mitk')
MITK_REGISTER_SERIALIZER(StringPropertySerializer);

#endif
