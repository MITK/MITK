/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-10-08 16:58:56 +0200 (Do, 08. Okt 2009) $
Version:   $Revision: 1.12 $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#ifndef mitkClippingPropertyDeserializer_h_included
#define mitkClippingPropertyDeserializer_h_included

#include "mitkBasePropertyDeserializer.h"
#include "mitkClippingProperty.h"
#include "mitkVector.h"

#include "SceneSerializationBaseExports.h"

namespace mitk
{

class SceneSerializationBase_EXPORT ClippingPropertyDeserializer : public BasePropertyDeserializer
{
  public:
    
    mitkClassMacro( ClippingPropertyDeserializer, BasePropertyDeserializer );
    itkNewMacro(Self);

    virtual BaseProperty::Pointer Deserialize(TiXmlElement* element)
    {
      if (!element) 
        return NULL;
      bool enabled = std::string(element->Attribute("enabled")) == "true";

      TiXmlElement* originElement = element->FirstChildElement("origin");
      if (originElement == NULL)
        return NULL;
      Point3D origin;
      if ( originElement->QueryFloatAttribute( "x", &origin[0] ) != TIXML_SUCCESS ) 
        return NULL;
      if ( originElement->QueryFloatAttribute( "y", &origin[1] ) != TIXML_SUCCESS ) 
        return NULL;
      if ( originElement->QueryFloatAttribute( "z", &origin[2] ) != TIXML_SUCCESS ) 
        return NULL;
      TiXmlElement* normalElement = element->FirstChildElement("normal");
      if (normalElement == NULL)
        return NULL;
      Vector3D normal;
      if ( normalElement->QueryFloatAttribute( "x", &normal[0] ) != TIXML_SUCCESS ) 
        return NULL;
      if ( normalElement->QueryFloatAttribute( "y", &normal[1] ) != TIXML_SUCCESS ) 
        return NULL;
      if ( normalElement->QueryFloatAttribute( "z", &normal[2] ) != TIXML_SUCCESS ) 
        return NULL;
      ClippingProperty::Pointer cp = ClippingProperty::New(origin, normal);
      cp->SetClippingEnabled(enabled);
     return cp.GetPointer();
    }
  protected:
    ClippingPropertyDeserializer() {}
    virtual ~ClippingPropertyDeserializer() {}
};
} // namespace
// important to put this into the GLOBAL namespace (because it starts with 'namespace mitk')
MITK_REGISTER_SERIALIZER(ClippingPropertyDeserializer);
#endif
