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

#ifndef mitkLevelWindowPropertySerializer_h_included
#define mitkLevelWindowPropertySerializer_h_included

#include "mitkBasePropertySerializer.h"
#include "mitkLevelWindowProperty.h"
#include <mitkLocaleSwitch.h>
#include <boost/lexical_cast.hpp>

namespace mitk
{

class LevelWindowPropertySerializer : public BasePropertySerializer
{
  public:

    mitkClassMacro( LevelWindowPropertySerializer, BasePropertySerializer );
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    virtual TiXmlElement* Serialize() override
    {
      if (const LevelWindowProperty* prop = dynamic_cast<const LevelWindowProperty*>(m_Property.GetPointer()))
      {
        LocaleSwitch localeSwitch("C");

        auto  element = new TiXmlElement("LevelWindow");

        LevelWindow lw = prop->GetLevelWindow();
        std::string boolString("false");
        if (lw.IsFixed() == true)
          boolString = "true";
        element->SetAttribute("fixed", boolString.c_str());

        std::string boolStringFltImage("false");
        if (lw.IsFloatingValues() == true)
          boolStringFltImage = "true";
        element->SetAttribute("isFloatingImage", boolStringFltImage.c_str());

        auto  child = new TiXmlElement("CurrentSettings");
        element->LinkEndChild( child );
          child->SetAttribute("level", boost::lexical_cast<std::string>(lw.GetLevel()));
          child->SetAttribute("window", boost::lexical_cast<std::string>(lw.GetWindow()));

                      child = new TiXmlElement("DefaultSettings");
        element->LinkEndChild( child );
          child->SetAttribute("level", boost::lexical_cast<std::string>(lw.GetDefaultLevel()));
          child->SetAttribute("window", boost::lexical_cast<std::string>(lw.GetDefaultWindow()));

                      child = new TiXmlElement("CurrentRange");
        element->LinkEndChild( child );
          child->SetAttribute("min", boost::lexical_cast<std::string>(lw.GetRangeMin()));
          child->SetAttribute("max", boost::lexical_cast<std::string>(lw.GetRangeMax()));


        return element;
      }
      else return nullptr;
    }

    virtual BaseProperty::Pointer Deserialize(TiXmlElement* element) override
    {
      if (!element) return nullptr;

      LocaleSwitch localeSwitch("C");

      bool isFixed(false);
      if (element->Attribute("fixed"))
        isFixed = std::string(element->Attribute("fixed")) == "true";
      bool isFloatingImage(false);
      if (element->Attribute("isFloatingImage"))
        isFloatingImage = std::string(element->Attribute("isFloatingImage")) == "true";

      std::string level_string;
      std::string window_string;
      TiXmlElement* child = element->FirstChildElement("CurrentSettings");
        if ( child->QueryStringAttribute( "level", &level_string ) != TIXML_SUCCESS ) return nullptr;
        if ( child->QueryStringAttribute( "window", &window_string ) != TIXML_SUCCESS ) return nullptr;

      std::string defaultLevel_string;
      std::string defaultWindow_string;
                    child = element->FirstChildElement("DefaultSettings");
        if ( child->QueryStringAttribute( "level", &defaultLevel_string ) != TIXML_SUCCESS ) return nullptr;
        if ( child->QueryStringAttribute( "window", &defaultWindow_string ) != TIXML_SUCCESS ) return nullptr;

      std::string minRange_string;
      std::string maxRange_string;
                    child = element->FirstChildElement("CurrentRange");
        if ( child->QueryStringAttribute( "min", &minRange_string ) != TIXML_SUCCESS ) return nullptr;
        if ( child->QueryStringAttribute( "max", &maxRange_string ) != TIXML_SUCCESS ) return nullptr;

      LevelWindow lw;
      try
      {
        lw.SetRangeMinMax( boost::lexical_cast<double>(minRange_string),
                           boost::lexical_cast<double>(maxRange_string) );
        lw.SetDefaultLevelWindow( boost::lexical_cast<double>(defaultLevel_string),
                                  boost::lexical_cast<double>(defaultWindow_string) );
        lw.SetLevelWindow( boost::lexical_cast<double>(level_string),
                           boost::lexical_cast<double>(window_string) );
        lw.SetFixed( isFixed );
        lw.SetFloatingValues(isFloatingImage);
      }
      catch ( boost::bad_lexical_cast& e )
      {
        MITK_ERROR << "Could not parse string as number: " << e.what();
        return nullptr;
      }
      return LevelWindowProperty::New( lw ).GetPointer();
    }

  protected:

    LevelWindowPropertySerializer() {}
    virtual ~LevelWindowPropertySerializer() {}
};

} // namespace

// important to put this into the GLOBAL namespace (because it starts with 'namespace mitk')
MITK_REGISTER_SERIALIZER(LevelWindowPropertySerializer);

#endif

