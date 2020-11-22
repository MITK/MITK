/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkVector3DPropertySerializer_h_included
#define mitkVector3DPropertySerializer_h_included

#include "mitkBasePropertySerializer.h"
#include "mitkProperties.h"
#include "mitkStringsToNumbers.h"
#include <mitkLocaleSwitch.h>

#include <array>

#include <tinyxml2.h>

namespace mitk
{
  class Vector3DPropertySerializer : public BasePropertySerializer
  {
  public:
    mitkClassMacro(Vector3DPropertySerializer, BasePropertySerializer)
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    tinyxml2::XMLElement* Serialize(tinyxml2::XMLDocument& doc) override
    {
      if (const Vector3DProperty *prop = dynamic_cast<const Vector3DProperty *>(m_Property.GetPointer()))
      {
        LocaleSwitch localeSwitch("C");

        auto *element = doc.NewElement("vector");
        Vector3D point = prop->GetValue();
        element->SetAttribute("x", boost::lexical_cast<std::string>(point[0]).c_str());
        element->SetAttribute("y", boost::lexical_cast<std::string>(point[1]).c_str());
        element->SetAttribute("z", boost::lexical_cast<std::string>(point[2]).c_str());
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

      std::array<const char*, 3> v_str = {
        element->Attribute("x"),
        element->Attribute("y"),
        element->Attribute("z")
      };
      if (nullptr == v_str[0] || nullptr == v_str[1] || nullptr == v_str[2])
        return nullptr;
      Vector3D v;
      try
      {
        StringsToNumbers<double>(3, v_str, v);
      }
      catch (boost::bad_lexical_cast &e)
      {
        MITK_ERROR << "Could not parse strings as numbers: " << e.what();
        return nullptr;
      }

      return Vector3DProperty::New(v).GetPointer();
    }

  protected:
    Vector3DPropertySerializer() {}
    ~Vector3DPropertySerializer() override {}
  };

} // namespace

// important to put this into the GLOBAL namespace (because it starts with 'namespace mitk')
MITK_REGISTER_SERIALIZER(Vector3DPropertySerializer);

#endif
