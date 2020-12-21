/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkFloatLookupTablePropertySerializer_h_included
#define mitkFloatLookupTablePropertySerializer_h_included

#include "mitkBasePropertySerializer.h"
#include "mitkProperties.h"
#include <mitkLexicalCast.h>
#include <mitkLocaleSwitch.h>
#include <tinyxml2.h>

namespace mitk
{
  class FloatLookupTablePropertySerializer : public BasePropertySerializer
  {
  public:
    mitkClassMacro(FloatLookupTablePropertySerializer, BasePropertySerializer)
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    tinyxml2::XMLElement *Serialize(tinyxml2::XMLDocument &doc) override
    {
      const FloatLookupTableProperty *prop = dynamic_cast<const FloatLookupTableProperty *>(m_Property.GetPointer());
      if (prop == nullptr)
        return nullptr;

      LocaleSwitch localeSwitch("C");

      FloatLookupTable lut = prop->GetValue();
      // if (lut.IsNull())
      //  return nullptr; // really?
      const FloatLookupTable::LookupTableType &map = lut.GetLookupTable();

      auto *element = doc.NewElement("FloatLookupTableTable");
      for (auto it = map.begin(); it != map.end(); ++it)
      {
        auto *tableEntry = doc.NewElement("LUTValue");
        tableEntry->SetAttribute("id", it->first);
        tableEntry->SetAttribute("value", boost::lexical_cast<std::string>(it->second).c_str());
        element->InsertEndChild(tableEntry);
      }
      return element;
    }

    BaseProperty::Pointer Deserialize(const tinyxml2::XMLElement *element) override
    {
      if (!element)
        return nullptr;

      LocaleSwitch localeSwitch("C");

      FloatLookupTable lut;
      for (auto *child = element->FirstChildElement("LUTValue"); child != nullptr;
           child = child->NextSiblingElement("LUTValue"))
      {
        int tempID;
        if (child->QueryIntAttribute("id", &tempID) != tinyxml2::XML_SUCCESS)
          return nullptr;
        FloatLookupTable::IdentifierType id = static_cast<FloatLookupTable::IdentifierType>(tempID);
        const char* value_string = child->Attribute("value");
        if (nullptr == value_string)
          return nullptr;
        try
        {
          lut.SetTableValue(id, boost::lexical_cast<float>(value_string));
        }
        catch (boost::bad_lexical_cast &e)
        {
          MITK_ERROR << "Could not parse string as number: " << e.what();
          return nullptr;
        }
      }
      return FloatLookupTableProperty::New(lut).GetPointer();
    }

  protected:
    FloatLookupTablePropertySerializer() {}
    ~FloatLookupTablePropertySerializer() override {}
  };
} // namespace
// important to put this into the GLOBAL namespace (because it starts with 'namespace mitk')
MITK_REGISTER_SERIALIZER(FloatLookupTablePropertySerializer);
#endif
