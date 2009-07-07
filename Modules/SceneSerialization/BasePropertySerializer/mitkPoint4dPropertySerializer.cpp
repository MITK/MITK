/*=========================================================================
 
Program:   Medical Imaging & Point4deraction Toolkit
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

#ifndef mitkPoint4dPropertySerializer_h_included
#define mitkPoint4dPropertySerializer_h_included

#include "mitkBasePropertySerializer.h"

#include "mitkProperties.h"

namespace mitk
{

class SceneSerialization_EXPORT Point4dPropertySerializer : public BasePropertySerializer
{
  public:
    
    mitkClassMacro( Point4dPropertySerializer, BasePropertySerializer );
    itkNewMacro(Self);

    virtual TiXmlElement* Serialize()
    {
      if (const Point4dProperty* prop = dynamic_cast<const Point4dProperty*>(m_Property.GetPointer()))
      {
        TiXmlElement* element = new TiXmlElement("point");
        Point4D point = prop->GetValue();
        element->SetAttribute("x", point[0]);
        element->SetAttribute("y", point[1]);
        element->SetAttribute("z", point[2]);
        element->SetAttribute("t", point[3]);
        return element;
      }
      else return NULL;
    }

  protected:

    Point4dPropertySerializer() {}
    virtual ~Point4dPropertySerializer() {}
};

} // namespace

// important to put this into the GLOBAL namespace (because it starts with 'namespace mitk')
MITK_REGISTER_SERIALIZER(Point4dPropertySerializer);

#endif

