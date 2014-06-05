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

#ifndef mitkClippingPropertySerializer_h_included
#define mitkClippingPropertySerializer_h_included

#include "mitkBasePropertySerializer.h"

#include "mitkClippingProperty.h"
#include "mitkNumericTypes.h"

#include <MitkSceneSerializationBaseExports.h>

namespace mitk
{
class MitkSceneSerializationBase_EXPORT ClippingPropertySerializer : public BasePropertySerializer
{
  public:
    mitkClassMacro( ClippingPropertySerializer, BasePropertySerializer );
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    virtual TiXmlElement* Serialize()
    {
      if (const ClippingProperty* prop = dynamic_cast<const ClippingProperty*>(m_Property.GetPointer()))
      {
        TiXmlElement* element = new TiXmlElement("clipping");
        if (prop->GetClippingEnabled())
          element->SetAttribute("enabled", "true");
        else
          element->SetAttribute("enabled", "false");
        TiXmlElement* originElement = new TiXmlElement("origin");
        const Point3D origin = prop->GetOrigin();
        originElement->SetDoubleAttribute("x", origin[0]);
        originElement->SetDoubleAttribute("y", origin[1]);
        originElement->SetDoubleAttribute("z", origin[2]);
        element->LinkEndChild(originElement);
        TiXmlElement* normalElement = new TiXmlElement("normal");
        const Vector3D normal = prop->GetNormal();
        normalElement->SetDoubleAttribute("x", normal[0]);
        normalElement->SetDoubleAttribute("y", normal[1]);
        normalElement->SetDoubleAttribute("z", normal[2]);
        element->LinkEndChild(normalElement);
        return element;
      }
      else
        return NULL;
    }

    virtual BaseProperty::Pointer Deserialize(TiXmlElement* element)
    {
      if (!element)
        return NULL;
      bool enabled = std::string(element->Attribute("enabled")) == "true";

      TiXmlElement* originElement = element->FirstChildElement("origin");
      if (originElement == NULL)
        return NULL;
      Point3D origin;
      if ( originElement->QueryDoubleAttribute( "x", &origin[0] ) != TIXML_SUCCESS )
        return NULL;
      if ( originElement->QueryDoubleAttribute( "y", &origin[1] ) != TIXML_SUCCESS )
        return NULL;
      if ( originElement->QueryDoubleAttribute( "z", &origin[2] ) != TIXML_SUCCESS )
        return NULL;
      TiXmlElement* normalElement = element->FirstChildElement("normal");
      if (normalElement == NULL)
        return NULL;
      Vector3D normal;
      if ( normalElement->QueryDoubleAttribute( "x", &normal[0] ) != TIXML_SUCCESS )
        return NULL;
      if ( normalElement->QueryDoubleAttribute( "y", &normal[1] ) != TIXML_SUCCESS )
        return NULL;
      if ( normalElement->QueryDoubleAttribute( "z", &normal[2] ) != TIXML_SUCCESS )
        return NULL;
      ClippingProperty::Pointer cp = ClippingProperty::New(origin, normal);
      cp->SetClippingEnabled(enabled);
     return cp.GetPointer();
    }
  protected:
    ClippingPropertySerializer() {}
    virtual ~ClippingPropertySerializer() {}
};
} // namespace
// important to put this into the GLOBAL namespace (because it starts with 'namespace mitk')
MITK_REGISTER_SERIALIZER(ClippingPropertySerializer);
#endif
