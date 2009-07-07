/*=========================================================================
 
Program:   Medical Imaging & Vector3Deraction Toolkit
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

#ifndef mitkVector3DPropertySerializer_h_included
#define mitkVector3DPropertySerializer_h_included

#include "mitkBasePropertySerializer.h"

#include "mitkProperties.h"

namespace mitk
{

class SceneSerialization_EXPORT Vector3DPropertySerializer : public BasePropertySerializer
{
  public:
    
    mitkClassMacro( Vector3DPropertySerializer, BasePropertySerializer );
    itkNewMacro(Self);

    virtual TiXmlElement* Serialize()
    {
      if (const Vector3DProperty* prop = dynamic_cast<const Vector3DProperty*>(m_Property.GetPointer()))
      {
        TiXmlElement* element = new TiXmlElement("vector");
        Vector3D point = prop->GetValue();
        element->SetAttribute("x", point[0]);
        element->SetAttribute("y", point[1]);
        element->SetAttribute("z", point[2]);
        return element;
      }
      else return NULL;
    }

  protected:

    Vector3DPropertySerializer() {}
    virtual ~Vector3DPropertySerializer() {}
};

} // namespace

// important to put this into the GLOBAL namespace (because it starts with 'namespace mitk')
MITK_REGISTER_SERIALIZER(Vector3DPropertySerializer);

#endif

