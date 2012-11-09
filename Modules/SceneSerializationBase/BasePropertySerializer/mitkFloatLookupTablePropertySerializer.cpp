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

#include "mitkProperties.h"

#include "SceneSerializationBaseExports.h"

namespace mitk
{

class SceneSerializationBase_EXPORT FloatLookupTablePropertySerializer : public BasePropertySerializer
{
  public:

    mitkClassMacro( FloatLookupTablePropertySerializer, BasePropertySerializer );
    itkNewMacro(Self);

    virtual TiXmlElement* Serialize()
    {
      const FloatLookupTableProperty* prop = dynamic_cast<const FloatLookupTableProperty*>(m_Property.GetPointer());
      if (prop == NULL)
        return NULL;
      FloatLookupTable lut = prop->GetValue();
      //if (lut.IsNull())
      //  return NULL; // really?
      const FloatLookupTable::LookupTableType& map = lut.GetLookupTable();

      TiXmlElement* element = new TiXmlElement("FloatLookupTableTable");
      for (FloatLookupTable::LookupTableType::const_iterator it = map.begin(); it != map.end(); ++it)
        {
          TiXmlElement* tableEntry = new TiXmlElement("LUTValue");
          tableEntry->SetAttribute("id", it->first);
          tableEntry->SetDoubleAttribute("value", static_cast<double>(it->second));
          element->LinkEndChild( tableEntry );
        }
        return element;
    }

    virtual BaseProperty::Pointer Deserialize(TiXmlElement* element)
    {
      if (!element)
        return NULL;

      FloatLookupTable lut;
      for( TiXmlElement* child = element->FirstChildElement("LUTValue"); child != NULL; child = child->NextSiblingElement("LUTValue"))
      {

        int tempID;
        if (child->QueryIntAttribute("id", &tempID) == TIXML_WRONG_TYPE)
          return NULL; // TODO: can we do a better error handling?
        FloatLookupTable::IdentifierType id = static_cast<FloatLookupTable::IdentifierType>(tempID);
        float tempVal = -1.0;
        if (child->QueryFloatAttribute("value", &tempVal) == TIXML_WRONG_TYPE)
          return NULL; // TODO: can we do a better error handling?
        FloatLookupTable::ValueType val = static_cast<FloatLookupTable::ValueType>(tempVal);
        lut.SetTableValue(id, val);
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
