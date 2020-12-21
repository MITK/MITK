/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkColorPropertySerializer_h_included
#define mitkColorPropertySerializer_h_included

#include "mitkBasePropertySerializer.h"
#include "mitkColorProperty.h"
#include "mitkStringsToNumbers.h"
#include <mitkLocaleSwitch.h>

#include <array>

#include <tinyxml2.h>

namespace mitk
{
  class ColorPropertySerializer : public BasePropertySerializer
  {
  public:
    mitkClassMacro(ColorPropertySerializer, BasePropertySerializer)
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    tinyxml2::XMLElement* Serialize(tinyxml2::XMLDocument& doc) override
    {
      if (const ColorProperty *prop = dynamic_cast<const ColorProperty *>(m_Property.GetPointer()))
      {
        LocaleSwitch localeSwitch("C");

        auto *element = doc.NewElement("color");
        Color color = prop->GetValue();
        element->SetAttribute("r", boost::lexical_cast<std::string>(color[0]).c_str());
        element->SetAttribute("g", boost::lexical_cast<std::string>(color[1]).c_str());
        element->SetAttribute("b", boost::lexical_cast<std::string>(color[2]).c_str());
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

      std::array<const char*, 3> c_string = {
        element->Attribute("r"),
        element->Attribute("g"),
        element->Attribute("b")
      };
      
      if (nullptr == c_string[0] || nullptr == c_string[1] || nullptr == c_string[2])
        return nullptr;

      Color c;
      try
      {
        StringsToNumbers<double>(3, c_string, c);
      }
      catch (boost::bad_lexical_cast &e)
      {
        MITK_ERROR << "Could not parse string as number: " << e.what();
        return nullptr;
      }

      return ColorProperty::New(c).GetPointer();
    }

  protected:
    ColorPropertySerializer() {}
    ~ColorPropertySerializer() override {}
  };

} // namespace

// important to put this into the GLOBAL namespace (because it starts with 'namespace mitk')
MITK_REGISTER_SERIALIZER(ColorPropertySerializer);

#endif
