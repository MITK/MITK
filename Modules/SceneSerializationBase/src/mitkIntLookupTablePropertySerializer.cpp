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

#ifndef mitkIntLookupTablePropertySerializer_h_included
#define mitkIntLookupTablePropertySerializer_h_included

#include "mitkBasePropertySerializer.h"

#include "mitkProperties.h"

namespace mitk
{

class IntLookupTablePropertySerializer : public BasePropertySerializer
{
  public:

    mitkClassMacro( IntLookupTablePropertySerializer, BasePropertySerializer );
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    virtual TiXmlElement* Serialize() override
    {
      const IntLookupTableProperty* prop = dynamic_cast<const IntLookupTableProperty*>(m_Property.GetPointer());
      if (prop == nullptr)
        return nullptr;
      IntLookupTable lut = prop->GetValue();
      //if (lut.IsNull())
      //  return NULL; // really?
      const IntLookupTable::LookupTableType& map = lut.GetLookupTable();

      auto  element = new TiXmlElement("IntLookupTableTable");
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

      IntLookupTable lut;
      for( TiXmlElement* child = element->FirstChildElement("LUTValue"); child != nullptr; child = child->NextSiblingElement("LUTValue"))
      {

        int temp;
        if (child->QueryIntAttribute("id", &temp) == TIXML_WRONG_TYPE)
          return nullptr; // TODO: can we do a better error handling?
        IntLookupTable::IdentifierType id = static_cast<IntLookupTable::IdentifierType>(temp);
        if (child->QueryIntAttribute("value", &temp) == TIXML_WRONG_TYPE)
          return nullptr; // TODO: can we do a better error handling?
        IntLookupTable::ValueType val = static_cast<IntLookupTable::ValueType>(temp);
        lut.SetTableValue(id, val);
      }
      return IntLookupTableProperty::New(lut).GetPointer();
    }
  protected:
    IntLookupTablePropertySerializer() {}
    virtual ~IntLookupTablePropertySerializer() {}
};
} // namespace
// important to put this into the GLOBAL namespace (because it starts with 'namespace mitk')
MITK_REGISTER_SERIALIZER(IntLookupTablePropertySerializer);
#endif
