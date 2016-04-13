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

#ifndef mitkFloatLookupTablePropertySerializer_h_included
#define mitkFloatLookupTablePropertySerializer_h_included

#include "mitkBasePropertySerializer.h"
#include <mitkLocaleSwitch.h>
#include "mitkProperties.h"
#include <boost/lexical_cast.hpp>

namespace mitk
{

class FloatLookupTablePropertySerializer : public BasePropertySerializer
{
  public:

    mitkClassMacro( FloatLookupTablePropertySerializer, BasePropertySerializer );
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    virtual TiXmlElement* Serialize() override
    {
      const FloatLookupTableProperty* prop = dynamic_cast<const FloatLookupTableProperty*>(m_Property.GetPointer());
      if (prop == nullptr)
        return nullptr;

      LocaleSwitch localeSwitch("C");

      FloatLookupTable lut = prop->GetValue();
      //if (lut.IsNull())
      //  return NULL; // really?
      const FloatLookupTable::LookupTableType& map = lut.GetLookupTable();

      auto  element = new TiXmlElement("FloatLookupTableTable");
      for (auto it = map.begin(); it != map.end(); ++it)
        {
          auto  tableEntry = new TiXmlElement("LUTValue");
          tableEntry->SetAttribute("id", it->first);
          tableEntry->SetAttribute("value", boost::lexical_cast<std::string>(it->second));
          element->LinkEndChild( tableEntry );
        }
        return element;
    }

    virtual BaseProperty::Pointer Deserialize(TiXmlElement* element) override
    {
      if (!element)
        return nullptr;

      LocaleSwitch localeSwitch("C");

      FloatLookupTable lut;
      for( TiXmlElement* child = element->FirstChildElement("LUTValue"); child != nullptr; child = child->NextSiblingElement("LUTValue"))
      {

        int tempID;
        if (child->QueryIntAttribute("id", &tempID) != TIXML_SUCCESS)
          return nullptr;
        FloatLookupTable::IdentifierType id = static_cast<FloatLookupTable::IdentifierType>(tempID);
        std::string value_string;
        if (child->QueryStringAttribute("value", &value_string) != TIXML_SUCCESS)
          return nullptr;
        try
        {
          lut.SetTableValue(id, boost::lexical_cast<float>(value_string));
        }
        catch (boost::bad_lexical_cast& e)
        {
          MITK_ERROR << "Could not parse string as number: " << e.what();
          return nullptr;
        }
      }
      return FloatLookupTableProperty::New(lut).GetPointer();
    }
  protected:
    FloatLookupTablePropertySerializer() {}
    virtual ~FloatLookupTablePropertySerializer() {}
};
} // namespace
// important to put this into the GLOBAL namespace (because it starts with 'namespace mitk')
MITK_REGISTER_SERIALIZER(FloatLookupTablePropertySerializer);
#endif
