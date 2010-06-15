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

#ifndef mitkIntLookupTablePropertyDeserializer_h_included
#define mitkIntLookupTablePropertyDeserializer_h_included

#include "mitkBasePropertyDeserializer.h"

#include "mitkProperties.h"

#include "SceneSerializationBaseExports.h"

namespace mitk
{

class SceneSerializationBase_EXPORT IntLookupTablePropertyDeserializer : public BasePropertyDeserializer
{
  public:
    
    mitkClassMacro( IntLookupTablePropertyDeserializer, BasePropertyDeserializer );
    itkNewMacro(Self);

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
    IntLookupTablePropertyDeserializer() {}
    virtual ~IntLookupTablePropertyDeserializer() {}
};
} // namespace
// important to put this into the GLOBAL namespace (because it starts with 'namespace mitk')
MITK_REGISTER_SERIALIZER(IntLookupTablePropertyDeserializer);
#endif
