/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkIntLookupTablePropertySerializer_h_included
#define mitkIntLookupTablePropertySerializer_h_included

#include "mitkBasePropertySerializer.h"
#include "mitkProperties.h"
#include <tinyxml2.h>

namespace mitk
{
  class IntLookupTablePropertySerializer : public BasePropertySerializer
  {
  public:
    mitkClassMacro(IntLookupTablePropertySerializer, BasePropertySerializer)
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    tinyxml2::XMLElement *Serialize(tinyxml2::XMLDocument &doc) override
    {
      const IntLookupTableProperty *prop = dynamic_cast<const IntLookupTableProperty *>(m_Property.GetPointer());
      if (prop == nullptr)
        return nullptr;
      IntLookupTable lut = prop->GetValue();
      // if (lut.IsNull())
      //  return nullptr; // really?
      const IntLookupTable::LookupTableType &map = lut.GetLookupTable();

      auto element = doc.NewElement("IntLookupTableTable");
      for (auto it = map.begin(); it != map.end(); ++it)
      {
        auto tableEntry = doc.NewElement("LUTValue");
        tableEntry->SetAttribute("id", it->first);
        tableEntry->SetAttribute("value", it->second);
        element->InsertEndChild(tableEntry);
      }
      return element;
    }

    BaseProperty::Pointer Deserialize(const tinyxml2::XMLElement *element) override
    {
      if (!element)
        return nullptr;

      IntLookupTable lut;
      for (auto *child = element->FirstChildElement("LUTValue"); child != nullptr;
           child = child->NextSiblingElement("LUTValue"))
      {
        int temp;
        if (child->QueryIntAttribute("id", &temp) != tinyxml2::XML_SUCCESS)
          return nullptr; // TODO: can we do a better error handling?
        IntLookupTable::IdentifierType id = static_cast<IntLookupTable::IdentifierType>(temp);
        if (child->QueryIntAttribute("value", &temp) != tinyxml2::XML_SUCCESS)
          return nullptr; // TODO: can we do a better error handling?
        IntLookupTable::ValueType val = static_cast<IntLookupTable::ValueType>(temp);
        lut.SetTableValue(id, val);
      }
      return IntLookupTableProperty::New(lut).GetPointer();
    }

  protected:
    IntLookupTablePropertySerializer() {}
    ~IntLookupTablePropertySerializer() override {}
  };
} // namespace
// important to put this into the GLOBAL namespace (because it starts with 'namespace mitk')
MITK_REGISTER_SERIALIZER(IntLookupTablePropertySerializer);
#endif
