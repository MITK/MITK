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

#ifndef mitkBoolLookupTablePropertyDeserializer_h_included
#define mitkBoolLookupTablePropertyDeserializer_h_included

#include "mitkBasePropertyDeserializer.h"

#include "mitkProperties.h"

namespace mitk
{

class SceneSerialization_EXPORT BoolLookupTablePropertyDeserializer : public BasePropertyDeserializer
{
  public:
    
    mitkClassMacro( BoolLookupTablePropertyDeserializer, BasePropertyDeserializer );
    itkNewMacro(Self);

    virtual BaseProperty::Pointer Deserialize(TiXmlElement* element)
    {
      if (!element) 
        return NULL;

      BoolLookupTable lut;
      for( TiXmlElement* child = element->FirstChildElement("LUTValue"); child != NULL; child = child->NextSiblingElement("LUTValue"))
      {

        int xmlID;
        if (child->QueryIntAttribute("id", &xmlID) == TIXML_WRONG_TYPE)
          return NULL; // TODO: can we do a better error handling?
        BoolLookupTable::IdentifierType id = static_cast<BoolLookupTable::IdentifierType>(xmlID);
        BoolLookupTable::ValueType val = std::string(child->Attribute("value")) == std::string("true");
        lut.SetTableValue(id, val);
      }
      return BoolLookupTableProperty::New(lut).GetPointer();
    }

  protected:
    BoolLookupTablePropertyDeserializer() {}
    virtual ~BoolLookupTablePropertyDeserializer() {}
};

} // namespace

// important to put this into the GLOBAL namespace (because it starts with 'namespace mitk')
MITK_REGISTER_SERIALIZER(BoolLookupTablePropertyDeserializer);

#endif

