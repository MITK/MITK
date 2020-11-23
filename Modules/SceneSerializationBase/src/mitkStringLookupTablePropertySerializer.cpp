/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkStringLookupTablePropertySerializer_h_included
#define mitkStringLookupTablePropertySerializer_h_included

#include "mitkBasePropertySerializer.h"
#include "mitkProperties.h"
#include <tinyxml2.h>

namespace mitk
{
  class StringLookupTablePropertySerializer : public BasePropertySerializer
  {
  public:
    mitkClassMacro(StringLookupTablePropertySerializer, BasePropertySerializer)
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    tinyxml2::XMLElement* Serialize(tinyxml2::XMLDocument& doc) override
    {
      const StringLookupTableProperty *prop = dynamic_cast<const StringLookupTableProperty *>(m_Property.GetPointer());
      if (prop == nullptr)
        return nullptr;
      StringLookupTable lut = prop->GetValue();
      // if (lut.IsNull())
      //  return nullptr; // really?
      const StringLookupTable::LookupTableType &map = lut.GetLookupTable();

      auto *element = doc.NewElement("StringLookupTable");
      for (auto it = map.begin(); it != map.end(); ++it)
      {
        auto *tableEntry = doc.NewElement("LUTValue");
        tableEntry->SetAttribute("id", it->first);
        tableEntry->SetAttribute("value", it->second.c_str());
        element->InsertEndChild(tableEntry);
      }
      return element;
    }

    BaseProperty::Pointer Deserialize(const tinyxml2::XMLElement *element) override
    {
      if (!element)
        return nullptr;

      StringLookupTable lut;
      for (auto *child = element->FirstChildElement("LUTValue"); child != nullptr;
           child = child->NextSiblingElement("LUTValue"))
      {
        int temp;
        if (child->QueryIntAttribute("id", &temp) != tinyxml2::XML_SUCCESS)
          return nullptr; // TODO: can we do a better error handling?
        StringLookupTable::IdentifierType id = static_cast<StringLookupTable::IdentifierType>(temp);

        if (child->Attribute("value") == nullptr)
          return nullptr; // TODO: can we do a better error handling?
        StringLookupTable::ValueType val = child->Attribute("value");
        lut.SetTableValue(id, val);
      }
      return StringLookupTableProperty::New(lut).GetPointer();
    }

  protected:
    StringLookupTablePropertySerializer() {}
    ~StringLookupTablePropertySerializer() override {}
  };
} // namespace
// important to put this into the GLOBAL namespace (because it starts with 'namespace mitk')
MITK_REGISTER_SERIALIZER(StringLookupTablePropertySerializer);
#endif
