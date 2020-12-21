/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkDoublePropertySerializer_h_included
#define mitkDoublePropertySerializer_h_included

#include "mitkBasePropertySerializer.h"

#include "mitkProperties.h"
#include <mitkLexicalCast.h>

#include <MitkSceneSerializationBaseExports.h>

#include <mitkLocaleSwitch.h>

#include <tinyxml2.h>

namespace mitk
{
  class DoublePropertySerializer : public BasePropertySerializer
  {
  public:
    mitkClassMacro(DoublePropertySerializer, BasePropertySerializer)
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    tinyxml2::XMLElement *Serialize(tinyxml2::XMLDocument &doc) override
    {
      if (const DoubleProperty *prop = dynamic_cast<const DoubleProperty *>(m_Property.GetPointer()))
      {
        LocaleSwitch localeSwitch("C");

        auto *element = doc.NewElement("double");
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

      const char* d = element->Attribute("value");
      if (nullptr != d)
      {
        try
        {
          return DoubleProperty::New(boost::lexical_cast<double>(d)).GetPointer();
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
    DoublePropertySerializer() {}
    ~DoublePropertySerializer() override {}
  };

} // namespace

// important to put this into the GLOBAL namespace (because it starts with 'namespace mitk')
MITK_REGISTER_SERIALIZER(DoublePropertySerializer);

#endif
