/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkClippingPropertySerializer_h_included
#define mitkClippingPropertySerializer_h_included

#include "mitkBasePropertySerializer.h"
#include "mitkClippingProperty.h"
#include "mitkNumericTypes.h"
#include <mitkLocaleSwitch.h>

#include "mitkStringsToNumbers.h"

#include <array>

#include <tinyxml2.h>

namespace mitk
{
  class ClippingPropertySerializer : public BasePropertySerializer
  {
  public:
    mitkClassMacro(ClippingPropertySerializer, BasePropertySerializer)
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    tinyxml2::XMLElement* Serialize(tinyxml2::XMLDocument& doc) override
    {
      if (const ClippingProperty *prop = dynamic_cast<const ClippingProperty *>(m_Property.GetPointer()))
      {
        LocaleSwitch localeSwitch("C");

        auto *element = doc.NewElement("clipping");
        element->SetAttribute("enabled", prop->GetClippingEnabled());
        auto *originElement = doc.NewElement("origin");
        const Point3D origin = prop->GetOrigin();
        originElement->SetAttribute("x", boost::lexical_cast<std::string>(origin[0]).c_str());
        originElement->SetAttribute("y", boost::lexical_cast<std::string>(origin[1]).c_str());
        originElement->SetAttribute("z", boost::lexical_cast<std::string>(origin[2]).c_str());
        element->InsertEndChild(originElement);
        auto *normalElement = doc.NewElement("normal");
        const Vector3D normal = prop->GetNormal();
        normalElement->SetAttribute("x", boost::lexical_cast<std::string>(normal[0]).c_str());
        normalElement->SetAttribute("y", boost::lexical_cast<std::string>(normal[1]).c_str());
        normalElement->SetAttribute("z", boost::lexical_cast<std::string>(normal[2]).c_str());
        element->InsertEndChild(normalElement);
        return element;
      }
      else
        return nullptr;
    }

    BaseProperty::Pointer Deserialize(const tinyxml2::XMLElement *element) override
    {
      if (!element)
        return nullptr;

      LocaleSwitch localeSwitch("C");

      bool enabled = std::string(element->Attribute("enabled")) == "true";

      auto *originElement = element->FirstChildElement("origin");
      if (originElement == nullptr)
        return nullptr;
      std::array<const char*, 3> origin_string = {
        originElement->Attribute("x"),
        originElement->Attribute("y"),
        originElement->Attribute("z")
      };
      if (nullptr == origin_string[0] || nullptr == origin_string[1] || nullptr == origin_string[2])
        return nullptr;
      Point3D origin;
      try
      {
        StringsToNumbers<ScalarType>(3, origin_string, origin);
      }
      catch (boost::bad_lexical_cast &e)
      {
        MITK_ERROR << "Could not parse string as number: " << e.what();
        return nullptr;
      }

      auto *normalElement = element->FirstChildElement("normal");
      if (normalElement == nullptr)
        return nullptr;
      std::array<const char*, 3> normal_string = {
        normalElement->Attribute("x"),
        normalElement->Attribute("y"),
        normalElement->Attribute("z")
      };
      if (nullptr == normal_string[0] || nullptr == normal_string[1] || nullptr == normal_string[2])
        return nullptr;
      Vector3D normal;
      try
      {
        StringsToNumbers<ScalarType>(3, normal_string, normal);
      }
      catch (boost::bad_lexical_cast &e)
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
    ~ClippingPropertySerializer() override {}
  };
} // namespace
// important to put this into the GLOBAL namespace (because it starts with 'namespace mitk')
MITK_REGISTER_SERIALIZER(ClippingPropertySerializer);
#endif
