/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkFloatPropertySerializer_h_included
#define mitkFloatPropertySerializer_h_included

#include "mitkBasePropertySerializer.h"

#include "mitkProperties.h"
#include "mitkStringsToNumbers.h"
#include <mitkLocaleSwitch.h>
#include <tinyxml2.h>

namespace mitk
{
  class FloatPropertySerializer : public BasePropertySerializer
  {
  public:
    mitkClassMacro(FloatPropertySerializer, BasePropertySerializer)
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    tinyxml2::XMLElement* Serialize(tinyxml2::XMLDocument& doc) override
    {
      if (const FloatProperty *prop = dynamic_cast<const FloatProperty *>(m_Property.GetPointer()))
      {
        LocaleSwitch localeSwitch("C");

        auto *element = doc.NewElement("float");
        element->SetAttribute("value", boost::lexical_cast<std::string>(prop->GetValue()).c_str());
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

      const char* f_string = element->Attribute("value");
      if (nullptr != f_string)
      {
        try
        {
          return FloatProperty::New(boost::lexical_cast<float>(f_string)).GetPointer();
        }
        catch (boost::bad_lexical_cast &e)
        {
          MITK_ERROR << "Could not parse string as number: " << e.what();
          return nullptr;
        }
      }
      else
      {
        return nullptr;
      }
    }

  protected:
    FloatPropertySerializer() {}
    ~FloatPropertySerializer() override {}
  };

} // namespace

// important to put this into the GLOBAL namespace (because it starts with 'namespace mitk')
MITK_REGISTER_SERIALIZER(FloatPropertySerializer);

#endif
