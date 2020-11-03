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

namespace mitk
{
  class FloatPropertySerializer : public BasePropertySerializer
  {
  public:
    mitkClassMacro(FloatPropertySerializer, BasePropertySerializer);
    itkFactorylessNewMacro(Self) itkCloneMacro(Self)

      TiXmlElement *Serialize() override
    {
      if (const FloatProperty *prop = dynamic_cast<const FloatProperty *>(m_Property.GetPointer()))
      {
        LocaleSwitch localeSwitch("C");

        auto element = new TiXmlElement("float");
        element->SetAttribute("value", boost::lexical_cast<std::string>(prop->GetValue()));
        return element;
      }
      else
        return nullptr;
    }

    BaseProperty::Pointer Deserialize(TiXmlElement *element) override
    {
      if (!element)
        return nullptr;

      LocaleSwitch localeSwitch("C");

      std::string f_string;
      if (element->QueryStringAttribute("value", &f_string) == TIXML_SUCCESS)
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
