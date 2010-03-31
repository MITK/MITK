/*=========================================================================
 
Program:   Medical Imaging & LookupTableeraction Toolkit
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

#ifndef mitkFloatLookupTablePropertyDeserializer_h_included
#define mitkFloatLookupTablePropertyDeserializer_h_included

#include "mitkBasePropertyDeserializer.h"

#include "mitkProperties.h"

namespace mitk
{

class SceneSerialization_EXPORT FloatLookupTablePropertyDeserializer : public BasePropertyDeserializer
{
  public:
    
    mitkClassMacro( FloatLookupTablePropertyDeserializer, BasePropertyDeserializer );
    itkNewMacro(Self);

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
    FloatLookupTablePropertyDeserializer() {}
    virtual ~FloatLookupTablePropertyDeserializer() {}
};
} // namespace
// important to put this into the GLOBAL namespace (because it starts with 'namespace mitk')
MITK_REGISTER_SERIALIZER(FloatLookupTablePropertyDeserializer);
#endif
