/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPoint4dPropertySerializer_h_included
#define mitkPoint4dPropertySerializer_h_included

#include "mitkBasePropertySerializer.h"
#include "mitkProperties.h"
#include "mitkStringsToNumbers.h"
#include <mitkLocaleSwitch.h>
#include <array>
#include <tinyxml2.h>

namespace mitk
{
  class Point4dPropertySerializer : public BasePropertySerializer
  {
  public:
    mitkClassMacro(Point4dPropertySerializer, BasePropertySerializer)
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    tinyxml2::XMLElement* Serialize(tinyxml2::XMLDocument& doc) override
    {
      if (const Point4dProperty *prop = dynamic_cast<const Point4dProperty *>(m_Property.GetPointer()))
      {
        LocaleSwitch localeSwitch("C");

        auto *element = doc.NewElement("point");
        Point4D point = prop->GetValue();
        element->SetAttribute("x", boost::lexical_cast<std::string>(point[0]).c_str());
        element->SetAttribute("y", boost::lexical_cast<std::string>(point[1]).c_str());
        element->SetAttribute("z", boost::lexical_cast<std::string>(point[2]).c_str());
        element->SetAttribute("t", boost::lexical_cast<std::string>(point[3]).c_str());
        return element;
      }
      else
        return nullptr;
    }

    BaseProperty::Pointer Deserialize(const tinyxml2::XMLElement* element) override
    {
      if (!element)
        return nullptr;

      LocaleSwitch localeSwitch("C");

      std::array<const char*, 4> v_str = {
        element->Attribute("x"),
        element->Attribute("y"),
        element->Attribute("z"),
        element->Attribute("t")
      };
      if(nullptr == v_str[0] || nullptr == v_str[1] || nullptr == v_str[2] || nullptr == v_str[2])
        return nullptr;
      Point4D v;
      try
      {
        StringsToNumbers<double>(4, v_str, v);
      }
      catch (boost::bad_lexical_cast &e)
      {
        MITK_ERROR << "Could not parse string as number: " << e.what();
        return nullptr;
      }
      return Point4dProperty::New(v).GetPointer();
    }

  protected:
    Point4dPropertySerializer() {}
    ~Point4dPropertySerializer() override {}
  };

} // namespace

// important to put this into the GLOBAL namespace (because it starts with 'namespace mitk')
MITK_REGISTER_SERIALIZER(Point4dPropertySerializer);

#endif
