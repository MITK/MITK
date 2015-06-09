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

#ifndef mitkBoolLookupTablePropertySerializer_h_included
#define mitkBoolLookupTablePropertySerializer_h_included

#include "mitkBasePropertySerializer.h"

#include "mitkProperties.h"

namespace mitk
{

class BoolLookupTablePropertySerializer : public BasePropertySerializer
{
  public:

    mitkClassMacro( BoolLookupTablePropertySerializer, BasePropertySerializer );
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    virtual TiXmlElement* Serialize() override
    {
      const BoolLookupTableProperty* prop = dynamic_cast<const BoolLookupTableProperty*>(m_Property.GetPointer());
      if (prop == nullptr)
        return nullptr;
      BoolLookupTable lut = prop->GetValue();
      //if (lut.IsNull())
      //  return NULL; // really?
      const BoolLookupTable::LookupTableType& map = lut.GetLookupTable();

      auto  element = new TiXmlElement("BoolLookupTable");
      for (auto it = map.begin(); it != map.end(); ++it)
        {
          auto  tableEntry = new TiXmlElement("LUTValue");
          tableEntry->SetAttribute("id", it->first);
          if (it->second == true)
            tableEntry->SetAttribute("value", "true");
          else
            tableEntry->SetAttribute("value", "false");
          element->LinkEndChild( tableEntry );
        }
        return element;
    }

    virtual BaseProperty::Pointer Deserialize(TiXmlElement* element) override
    {
      if (!element)
        return nullptr;

      BoolLookupTable lut;
      for( TiXmlElement* child = element->FirstChildElement("LUTValue"); child != nullptr; child = child->NextSiblingElement("LUTValue"))
      {

        int xmlID;
        if (child->QueryIntAttribute("id", &xmlID) == TIXML_WRONG_TYPE)
          return nullptr; // TODO: can we do a better error handling?
        BoolLookupTable::IdentifierType id = static_cast<BoolLookupTable::IdentifierType>(xmlID);
        BoolLookupTable::ValueType val = std::string(child->Attribute("value")) == std::string("true");
        lut.SetTableValue(id, val);
      }
      return BoolLookupTableProperty::New(lut).GetPointer();
    }
  protected:
    BoolLookupTablePropertySerializer() {}
    virtual ~BoolLookupTablePropertySerializer() {}
};
} // namespace
// important to put this into the GLOBAL namespace (because it starts with 'namespace mitk')
MITK_REGISTER_SERIALIZER(BoolLookupTablePropertySerializer);
#endif
