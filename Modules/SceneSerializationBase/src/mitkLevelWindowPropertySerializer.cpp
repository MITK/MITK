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
          child->SetDoubleAttribute("level", lw.GetLevel());
          child->SetDoubleAttribute("window", lw.GetWindow());

                      child = new TiXmlElement("DefaultSettings");
        element->LinkEndChild( child );
          child->SetDoubleAttribute("level", lw.GetDefaultLevel());
          child->SetDoubleAttribute("window", lw.GetDefaultWindow());

                      child = new TiXmlElement("CurrentRange");
        element->LinkEndChild( child );
          child->SetDoubleAttribute("min", lw.GetRangeMin());
          child->SetDoubleAttribute("max", lw.GetRangeMax());


        return element;
      }
      else return nullptr;
    }

    virtual BaseProperty::Pointer Deserialize(TiXmlElement* element) override
    {
      if (!element) return nullptr;

      bool isFixed(false);
      if (element->Attribute("fixed"))
        isFixed = std::string(element->Attribute("fixed")) == "true";
      bool isFloatingImage(false);
      if (element->Attribute("isFloatingImage"))
        isFloatingImage = std::string(element->Attribute("isFloatingImage")) == "true";

      double level = 0;
      double window = 0;
      TiXmlElement* child = element->FirstChildElement("CurrentSettings");
        if ( child->QueryDoubleAttribute( "level", &level ) != TIXML_SUCCESS ) return nullptr;
        if ( child->QueryDoubleAttribute( "window", &window ) != TIXML_SUCCESS ) return nullptr;

      double defaultLevel;
      double defaultWindow;
                    child = element->FirstChildElement("DefaultSettings");
        if ( child->QueryDoubleAttribute( "level", &defaultLevel ) != TIXML_SUCCESS ) return nullptr;
        if ( child->QueryDoubleAttribute( "window", &defaultWindow ) != TIXML_SUCCESS ) return nullptr;

      double minRange;
      double maxRange;
                    child = element->FirstChildElement("CurrentRange");
        if ( child->QueryDoubleAttribute( "min", &minRange ) != TIXML_SUCCESS ) return nullptr;
        if ( child->QueryDoubleAttribute( "max", &maxRange ) != TIXML_SUCCESS ) return nullptr;

      LevelWindow lw;
      lw.SetRangeMinMax( minRange, maxRange );
      lw.SetDefaultLevelWindow( defaultLevel, defaultWindow );
      lw.SetLevelWindow( level, window );
      lw.SetFixed( isFixed );
      lw.SetFloatingValues(isFloatingImage);

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

