/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-10-08 17:07:40 +0200 (Do, 08. Okt 2009) $
Version:   $Revision: 1.12 $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#ifndef mitkStringLookupTablePropertyDeserializer_h_included
#define mitkStringLookupTablePropertyDeserializer_h_included

#include "mitkBasePropertyDeserializer.h"

#include "mitkProperties.h"

#include "SceneSerializationExports.h"

namespace mitk
{

class SceneSerialization_EXPORT StringLookupTablePropertyDeserializer : public BasePropertyDeserializer
{
  public:
    
    mitkClassMacro( StringLookupTablePropertyDeserializer, BasePropertyDeserializer );
    itkNewMacro(Self);

    virtual BaseProperty::Pointer Deserialize(TiXmlElement* element)
    {
      if (!element) 
        return NULL;

      StringLookupTable lut;
      for( TiXmlElement* child = element->FirstChildElement("LUTValue"); child != NULL; child = child->NextSiblingElement("LUTValue"))
      {

        int temp;
        if (child->QueryIntAttribute("id", &temp) == TIXML_WRONG_TYPE)
          return NULL; // TODO: can we do a better error handling?
        StringLookupTable::IdentifierType id = static_cast<StringLookupTable::IdentifierType>(temp);
        
        if (child->Attribute("value") == NULL)
          return NULL; // TODO: can we do a better error handling?
        StringLookupTable::ValueType val = child->Attribute("value");
        lut.SetTableValue(id, val);
      }
      return StringLookupTableProperty::New(lut).GetPointer();
    }
  protected:
    StringLookupTablePropertyDeserializer() {}
    virtual ~StringLookupTablePropertyDeserializer() {}
};
} // namespace
// important to put this into the GLOBAL namespace (because it starts with 'namespace mitk')
MITK_REGISTER_SERIALIZER(StringLookupTablePropertyDeserializer);
#endif
