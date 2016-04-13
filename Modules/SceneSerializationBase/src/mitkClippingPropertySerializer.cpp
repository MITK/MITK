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
#include <mitkLocaleSwitch.h>
#include "mitkClippingProperty.h"
#include "mitkNumericTypes.h"

#include "mitkStringsToNumbers.h"

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
        LocaleSwitch localeSwitch("C");

        auto  element = new TiXmlElement("clipping");
        if (prop->GetClippingEnabled())
          element->SetAttribute("enabled", "true");
        else
          element->SetAttribute("enabled", "false");
        auto  originElement = new TiXmlElement("origin");
        const Point3D origin = prop->GetOrigin();
        originElement->SetAttribute("x", boost::lexical_cast<std::string>(origin[0]));
        originElement->SetAttribute("y", boost::lexical_cast<std::string>(origin[1]));
        originElement->SetAttribute("z", boost::lexical_cast<std::string>(origin[2]));
        element->LinkEndChild(originElement);
        auto  normalElement = new TiXmlElement("normal");
        const Vector3D normal = prop->GetNormal();
        normalElement->SetAttribute("x", boost::lexical_cast<std::string>(normal[0]));
        normalElement->SetAttribute("y", boost::lexical_cast<std::string>(normal[1]));
        normalElement->SetAttribute("z", boost::lexical_cast<std::string>(normal[2]));
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

      LocaleSwitch localeSwitch("C");

      bool enabled = std::string(element->Attribute("enabled")) == "true";

      TiXmlElement* originElement = element->FirstChildElement("origin");
      if (originElement == nullptr)
        return nullptr;
      std::string origin_string[3];
      if ( originElement->QueryStringAttribute( "x", &origin_string[0] ) != TIXML_SUCCESS )
        return nullptr;
      if ( originElement->QueryStringAttribute( "y", &origin_string[1] ) != TIXML_SUCCESS )
        return nullptr;
      if ( originElement->QueryStringAttribute( "z", &origin_string[2] ) != TIXML_SUCCESS )
        return nullptr;
      Point3D origin;
      try
      {
        StringsToNumbers<ScalarType>(3, origin_string, origin);
      }
      catch (boost::bad_lexical_cast& e)
      {
          MITK_ERROR << "Could not parse string as number: " << e.what();
          return nullptr;
      }

      TiXmlElement* normalElement = element->FirstChildElement("normal");
      if (normalElement == nullptr)
        return nullptr;
      std::string normal_string[3];
      if ( normalElement->QueryStringAttribute( "x", &normal_string[0] ) != TIXML_SUCCESS )
        return nullptr;
      if ( normalElement->QueryStringAttribute( "y", &normal_string[1] ) != TIXML_SUCCESS )
        return nullptr;
      if ( normalElement->QueryStringAttribute( "z", &normal_string[2] ) != TIXML_SUCCESS )
        return nullptr;
      Vector3D normal;
      try
      {
        StringsToNumbers<ScalarType>(3, normal_string, normal);
      }
      catch (boost::bad_lexical_cast& e)
      {
          MITK_ERROR << "Could not parse string as number: " << e.what();
          return nullptr;
      }

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
