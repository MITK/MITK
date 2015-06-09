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

namespace mitk
{
class ClippingPropertySerializer : public BasePropertySerializer
{
  public:
    mitkClassMacro( ClippingPropertySerializer, BasePropertySerializer );
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    virtual TiXmlElement* Serialize() override
    {
      if (const ClippingProperty* prop = dynamic_cast<const ClippingProperty*>(m_Property.GetPointer()))
      {
        auto  element = new TiXmlElement("clipping");
        if (prop->GetClippingEnabled())
          element->SetAttribute("enabled", "true");
        else
          element->SetAttribute("enabled", "false");
        auto  originElement = new TiXmlElement("origin");
        const Point3D origin = prop->GetOrigin();
        originElement->SetDoubleAttribute("x", origin[0]);
        originElement->SetDoubleAttribute("y", origin[1]);
        originElement->SetDoubleAttribute("z", origin[2]);
        element->LinkEndChild(originElement);
        auto  normalElement = new TiXmlElement("normal");
        const Vector3D normal = prop->GetNormal();
        normalElement->SetDoubleAttribute("x", normal[0]);
        normalElement->SetDoubleAttribute("y", normal[1]);
        normalElement->SetDoubleAttribute("z", normal[2]);
        element->LinkEndChild(normalElement);
        return element;
      }
      else
        return nullptr;
    }

    virtual BaseProperty::Pointer Deserialize(TiXmlElement* element) override
    {
      if (!element)
        return nullptr;
      bool enabled = std::string(element->Attribute("enabled")) == "true";

      TiXmlElement* originElement = element->FirstChildElement("origin");
      if (originElement == nullptr)
        return nullptr;
      Point3D origin;
      if ( originElement->QueryDoubleAttribute( "x", &origin[0] ) != TIXML_SUCCESS )
        return nullptr;
      if ( originElement->QueryDoubleAttribute( "y", &origin[1] ) != TIXML_SUCCESS )
        return nullptr;
      if ( originElement->QueryDoubleAttribute( "z", &origin[2] ) != TIXML_SUCCESS )
        return nullptr;
      TiXmlElement* normalElement = element->FirstChildElement("normal");
      if (normalElement == nullptr)
        return nullptr;
      Vector3D normal;
      if ( normalElement->QueryDoubleAttribute( "x", &normal[0] ) != TIXML_SUCCESS )
        return nullptr;
      if ( normalElement->QueryDoubleAttribute( "y", &normal[1] ) != TIXML_SUCCESS )
        return nullptr;
      if ( normalElement->QueryDoubleAttribute( "z", &normal[2] ) != TIXML_SUCCESS )
        return nullptr;
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
