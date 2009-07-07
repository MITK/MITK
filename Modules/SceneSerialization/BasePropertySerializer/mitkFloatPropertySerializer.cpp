/*=========================================================================
 
Program:   Medical Imaging & Floateraction Toolkit
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

#ifndef mitkFloatPropertySerializer_h_included
#define mitkFloatPropertySerializer_h_included

#include "mitkBasePropertySerializer.h"

#include "mitkProperties.h"

namespace mitk
{

class SceneSerialization_EXPORT FloatPropertySerializer : public BasePropertySerializer
{
  public:
    
    mitkClassMacro( FloatPropertySerializer, BasePropertySerializer );
    itkNewMacro(Self);

    virtual TiXmlElement* Serialize()
    {
      if (const FloatProperty* prop = dynamic_cast<const FloatProperty*>(m_Property.GetPointer()))
      {
        TiXmlElement* element = new TiXmlElement("float");
        element->SetDoubleAttribute("value", static_cast<double>(prop->GetValue()));
        return element;
      }
      else return NULL;
    }

  protected:

    FloatPropertySerializer() {}
    virtual ~FloatPropertySerializer() {}
};

} // namespace

// important to put this into the GLOBAL namespace (because it starts with 'namespace mitk')
MITK_REGISTER_SERIALIZER(FloatPropertySerializer);

#endif

