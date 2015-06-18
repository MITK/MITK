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

#ifndef mitkStringLookupTablePropertySerializer_h_included
#define mitkStringLookupTablePropertySerializer_h_included

#include "mitkBasePropertySerializer.h"

#include "mitkProperties.h"

namespace mitk
{

class StringLookupTablePropertySerializer : public BasePropertySerializer
{
  public:

    mitkClassMacro( StringLookupTablePropertySerializer, BasePropertySerializer );
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    virtual TiXmlElement* Serialize() override
    {
      const StringLookupTableProperty* prop = dynamic_cast<const StringLookupTableProperty*>(m_Property.GetPointer());
      if (prop == nullptr)
        return nullptr;
      StringLookupTable lut = prop->GetValue();
      //if (lut.IsNull())
      //  return NULL; // really?
      const StringLookupTable::LookupTableType& map = lut.GetLookupTable();

      auto  element = new TiXmlElement("StringLookupTable");
      for (auto it = map.begin(); it != map.end(); ++it)
        {
          auto  tableEntry = new TiXmlElement("LUTValue");
          tableEntry->SetAttribute("id", it->first);
          tableEntry->SetAttribute("value", it->second);
          element->LinkEndChild( tableEntry );
        }
        return element;
    }

    virtual BaseProperty::Pointer Deserialize(TiXmlElement* element) override
    {
      if (!element)
        return nullptr;

      StringLookupTable lut;
      for( TiXmlElement* child = element->FirstChildElement("LUTValue"); child != nullptr; child = child->NextSiblingElement("LUTValue"))
      {

        int temp;
        if (child->QueryIntAttribute("id", &temp) == TIXML_WRONG_TYPE)
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
    virtual ~StringLookupTablePropertySerializer() {}
};
} // namespace
// important to put this into the GLOBAL namespace (because it starts with 'namespace mitk')
MITK_REGISTER_SERIALIZER(StringLookupTablePropertySerializer);
#endif
