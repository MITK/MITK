/*=========================================================================
 
Program:   Medical Imaging & LevelWindoweraction Toolkit
Module:    $RCSfile: mitkPropertyManager.cpp,v $
Language:  C++
Date:      $Date$
Version:   $Revision: 1.12 $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#ifndef mitkLevelWindowPropertyDeserializer_h_included
#define mitkLevelWindowPropertyDeserializer_h_included

#include "mitkBasePropertyDeserializer.h"

#include "mitkLevelWindowProperty.h"

namespace mitk
{

class SceneSerialization_EXPORT LevelWindowPropertyDeserializer : public BasePropertyDeserializer
{
  public:
    
    mitkClassMacro( LevelWindowPropertyDeserializer, BasePropertyDeserializer );
    itkNewMacro(Self);

    virtual BaseProperty::Pointer Deserialize(TiXmlElement* element)
    {
      if (!element) return NULL;

      bool isFixed(false);
      if (element->Attribute("fixed"))
        isFixed = std::string(element->Attribute("fixed")) == "true";

      float level;
      float window;
      TiXmlElement* child = element->FirstChildElement("CurrentSettings");
        if ( child->QueryFloatAttribute( "level", &level ) != TIXML_SUCCESS ) return NULL;
        if ( child->QueryFloatAttribute( "window", &window ) != TIXML_SUCCESS ) return NULL;

      float defaultLevel;
      float defaultWindow;
                    child = element->FirstChildElement("DefaultSettings");
        if ( child->QueryFloatAttribute( "level", &defaultLevel ) != TIXML_SUCCESS ) return NULL;
        if ( child->QueryFloatAttribute( "window", &defaultWindow ) != TIXML_SUCCESS ) return NULL;

      float minRange;
      float maxRange;
                    child = element->FirstChildElement("CurrentRange");
        if ( child->QueryFloatAttribute( "min", &minRange ) != TIXML_SUCCESS ) return NULL;
        if ( child->QueryFloatAttribute( "max", &maxRange ) != TIXML_SUCCESS ) return NULL;


        
      LevelWindow lw;
      lw.SetRangeMinMax( minRange, maxRange );
      lw.SetDefaultLevelWindow( defaultLevel, defaultWindow );
      lw.SetLevelWindow( level, window );
      lw.SetFixed( isFixed );

      return LevelWindowProperty::New( lw ).GetPointer();
    }

  protected:

    LevelWindowPropertyDeserializer() {}
    virtual ~LevelWindowPropertyDeserializer() {}
};

} // namespace

// important to put this into the GLOBAL namespace (because it starts with 'namespace mitk')
MITK_REGISTER_SERIALIZER(LevelWindowPropertyDeserializer);

#endif

