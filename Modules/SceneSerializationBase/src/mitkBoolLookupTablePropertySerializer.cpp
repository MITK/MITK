/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkBoolLookupTablePropertySerializer_h_included
#define mitkBoolLookupTablePropertySerializer_h_included

#include "mitkBasePropertySerializer.h"

#include "mitkProperties.h"

#include <tinyxml2.h>

namespace mitk
{
  class BoolLookupTablePropertySerializer : public BasePropertySerializer
  {
  public:
    mitkClassMacro(BoolLookupTablePropertySerializer, BasePropertySerializer)
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    tinyxml2::XMLElement *Serialize(tinyxml2::XMLDocument &doc) override
    {
      const BoolLookupTableProperty *prop = dynamic_cast<const BoolLookupTableProperty *>(m_Property.GetPointer());
      if (prop == nullptr)
        return nullptr;
      BoolLookupTable lut = prop->GetValue();
      // if (lut.IsNull())
      //  return nullptr; // really?
      const BoolLookupTable::LookupTableType &map = lut.GetLookupTable();

      auto *element = doc.NewElement("BoolLookupTable");
      for (auto it = map.begin(); it != map.end(); ++it)
      {
        auto *tableEntry = doc.NewElement("LUTValue");
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

      BoolLookupTable lut;
      for (auto *child = element->FirstChildElement("LUTValue"); child != nullptr;
           child = child->NextSiblingElement("LUTValue"))
      {
        int xmlID;
        if (child->QueryIntAttribute("id", &xmlID) != tinyxml2::XML_SUCCESS)
          return nullptr; // TODO: can we do a better error handling?
        BoolLookupTable::IdentifierType id = static_cast<BoolLookupTable::IdentifierType>(xmlID);
        BoolLookupTable::ValueType val = std::string(child->Attribute("value")) == std::string("true");
        lut.SetTableValue(id, val);
      }
      return BoolLookupTableProperty::New(lut).GetPointer();
    }

  protected:
    BoolLookupTablePropertySerializer() {}
    ~BoolLookupTablePropertySerializer() override {}
  };
} // namespace
// important to put this into the GLOBAL namespace (because it starts with 'namespace mitk')
MITK_REGISTER_SERIALIZER(BoolLookupTablePropertySerializer);
#endif
