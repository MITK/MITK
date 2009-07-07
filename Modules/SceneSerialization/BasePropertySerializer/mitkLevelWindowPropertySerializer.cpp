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

#ifndef mitkLevelWindowPropertySerializer_h_included
#define mitkLevelWindowPropertySerializer_h_included

#include "mitkBasePropertySerializer.h"

#include "mitkLevelWindowProperty.h"

namespace mitk
{

class SceneSerialization_EXPORT LevelWindowPropertySerializer : public BasePropertySerializer
{
  public:
    
    mitkClassMacro( LevelWindowPropertySerializer, BasePropertySerializer );
    itkNewMacro(Self);

    virtual TiXmlElement* Serialize()
    {
      if (const LevelWindowProperty* prop = dynamic_cast<const LevelWindowProperty*>(m_Property.GetPointer()))
      {
        TiXmlElement* element = new TiXmlElement("LevelWindow");

        LevelWindow lw = prop->GetLevelWindow();
        std::string boolString("false");
        if (lw.IsFixed() == true)
          boolString = "true";
        element->SetAttribute("fixed", boolString.c_str());

        TiXmlElement* child = new TiXmlElement("CurrentSettings");
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

                child = new TiXmlElement("DefaultRange");
        element->LinkEndChild( child );
          child->SetDoubleAttribute("min", lw.GetDefaultRangeMin());
          child->SetDoubleAttribute("max", lw.GetDefaultRangeMax());

        return element;
      }
      else return NULL;
    }

  protected:

    LevelWindowPropertySerializer() {}
    virtual ~LevelWindowPropertySerializer() {}
};

} // namespace

// important to put this into the GLOBAL namespace (because it starts with 'namespace mitk')
MITK_REGISTER_SERIALIZER(LevelWindowPropertySerializer);

#endif

