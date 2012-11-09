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

#include "SceneSerializationBaseExports.h"

namespace mitk
{

class SceneSerializationBase_EXPORT IntLookupTablePropertySerializer : public BasePropertySerializer
{
  public:

    mitkClassMacro( IntLookupTablePropertySerializer, BasePropertySerializer );
    itkNewMacro(Self);

    virtual TiXmlElement* Serialize()
    {
      const IntLookupTableProperty* prop = dynamic_cast<const IntLookupTableProperty*>(m_Property.GetPointer());
      if (prop == NULL)
        return NULL;
      IntLookupTable lut = prop->GetValue();
      //if (lut.IsNull())
      //  return NULL; // really?
      const IntLookupTable::LookupTableType& map = lut.GetLookupTable();

      TiXmlElement* element = new TiXmlElement("IntLookupTableTable");
      for (IntLookupTable::LookupTableType::const_iterator it = map.begin(); it != map.end(); ++it)
        {
          TiXmlElement* tableEntry = new TiXmlElement("LUTValue");
          tableEntry->SetAttribute("id", it->first);
          tableEntry->SetAttribute("value", it->second);
          element->LinkEndChild( tableEntry );
        }
        return element;
    }

    virtual BaseProperty::Pointer Deserialize(TiXmlElement* element)
    {
      if (!element)
        return NULL;

      IntLookupTable lut;
      for( TiXmlElement* child = element->FirstChildElement("LUTValue"); child != NULL; child = child->NextSiblingElement("LUTValue"))
      {

        int temp;
        if (child->QueryIntAttribute("id", &temp) == TIXML_WRONG_TYPE)
          return NULL; // TODO: can we do a better error handling?
        IntLookupTable::IdentifierType id = static_cast<IntLookupTable::IdentifierType>(temp);
        if (child->QueryIntAttribute("value", &temp) == TIXML_WRONG_TYPE)
          return NULL; // TODO: can we do a better error handling?
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
