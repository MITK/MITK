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

      float level;
      float window;
      TiXmlElement* child = element->FirstChildElement("CurrentSettings");
        if ( child->QueryFloatAttribute( "level", &level ) != TIXML_SUCCESS ) return nullptr;
        if ( child->QueryFloatAttribute( "window", &window ) != TIXML_SUCCESS ) return nullptr;

      float defaultLevel;
      float defaultWindow;
                    child = element->FirstChildElement("DefaultSettings");
        if ( child->QueryFloatAttribute( "level", &defaultLevel ) != TIXML_SUCCESS ) return nullptr;
        if ( child->QueryFloatAttribute( "window", &defaultWindow ) != TIXML_SUCCESS ) return nullptr;

      float minRange;
      float maxRange;
                    child = element->FirstChildElement("CurrentRange");
        if ( child->QueryFloatAttribute( "min", &minRange ) != TIXML_SUCCESS ) return nullptr;
        if ( child->QueryFloatAttribute( "max", &maxRange ) != TIXML_SUCCESS ) return nullptr;



      LevelWindow lw;
      lw.SetRangeMinMax( minRange, maxRange );
      lw.SetDefaultLevelWindow( defaultLevel, defaultWindow );
      lw.SetLevelWindow( level, window );
      lw.SetFixed( isFixed );

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

