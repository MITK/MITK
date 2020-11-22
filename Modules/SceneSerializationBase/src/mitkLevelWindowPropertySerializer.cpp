/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkLevelWindowPropertySerializer_h_included
#define mitkLevelWindowPropertySerializer_h_included

#include "mitkBasePropertySerializer.h"
#include "mitkLevelWindowProperty.h"
#include <mitkLexicalCast.h>
#include <mitkLocaleSwitch.h>
#include <tinyxml2.h>

namespace mitk
{
  class LevelWindowPropertySerializer : public BasePropertySerializer
  {
  public:
    mitkClassMacro(LevelWindowPropertySerializer, BasePropertySerializer)
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    tinyxml2::XMLElement *Serialize(tinyxml2::XMLDocument &doc) override
    {
      if (const LevelWindowProperty *prop = dynamic_cast<const LevelWindowProperty *>(m_Property.GetPointer()))
      {
        LocaleSwitch localeSwitch("C");

        auto *element = doc.NewElement("LevelWindow");

        LevelWindow lw = prop->GetLevelWindow();
        element->SetAttribute("fixed", lw.IsFixed());
        element->SetAttribute("isFloatingImage", lw.IsFloatingValues());

        auto *child = doc.NewElement("CurrentSettings");
        element->InsertEndChild(child);
        child->SetAttribute("level", boost::lexical_cast<std::string>(lw.GetLevel()).c_str());
        child->SetAttribute("window", boost::lexical_cast<std::string>(lw.GetWindow()).c_str());

        child = doc.NewElement("DefaultSettings");
        element->InsertEndChild(child);
        child->SetAttribute("level", boost::lexical_cast<std::string>(lw.GetDefaultLevel()).c_str());
        child->SetAttribute("window", boost::lexical_cast<std::string>(lw.GetDefaultWindow()).c_str());

        child = doc.NewElement("CurrentRange");
        element->InsertEndChild(child);
        child->SetAttribute("min", boost::lexical_cast<std::string>(lw.GetRangeMin()).c_str());
        child->SetAttribute("max", boost::lexical_cast<std::string>(lw.GetRangeMax()).c_str());

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

      bool isFixed(false);
      if (element->Attribute("fixed"))
        isFixed = std::string(element->Attribute("fixed")) == "true";
      bool isFloatingImage(false);
      if (element->Attribute("isFloatingImage"))
        isFloatingImage = std::string(element->Attribute("isFloatingImage")) == "true";

      auto *child = element->FirstChildElement("CurrentSettings");
      const char* level_string = child->Attribute("level");
      const char* window_string = child->Attribute("window");
      if (nullptr == level_string || nullptr == window_string)
        return nullptr;

      child = element->FirstChildElement("DefaultSettings");
      const char* defaultLevel_string = child->Attribute("level");
      const char* defaultWindow_string = child->Attribute("window");
      if (nullptr == defaultLevel_string || nullptr == defaultWindow_string)
        return nullptr;

      child = element->FirstChildElement("CurrentRange");
      const char* minRange_string = child->Attribute("min");
      const char* maxRange_string = child->Attribute("max");
      if (nullptr == minRange_string || nullptr == maxRange_string)
        return nullptr;

      LevelWindow lw;
      try
      {
        lw.SetRangeMinMax(boost::lexical_cast<double>(minRange_string), boost::lexical_cast<double>(maxRange_string));
        lw.SetDefaultLevelWindow(boost::lexical_cast<double>(defaultLevel_string),
                                 boost::lexical_cast<double>(defaultWindow_string));
        lw.SetLevelWindow(boost::lexical_cast<double>(level_string), boost::lexical_cast<double>(window_string));
        lw.SetFixed(isFixed);
        lw.SetFloatingValues(isFloatingImage);
      }
      catch (boost::bad_lexical_cast &e)
      {
        MITK_ERROR << "Could not parse string as number: " << e.what();
        return nullptr;
      }
      return LevelWindowProperty::New(lw).GetPointer();
    }

  protected:
    LevelWindowPropertySerializer() {}
    ~LevelWindowPropertySerializer() override {}
  };

} // namespace

// important to put this into the GLOBAL namespace (because it starts with 'namespace mitk')
MITK_REGISTER_SERIALIZER(LevelWindowPropertySerializer);

#endif
