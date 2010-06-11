/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
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

#ifndef mitkPoint3dPropertySerializer_h_included
#define mitkPoint3dPropertySerializer_h_included

#include "mitkBasePropertySerializer.h"

#include "mitkProperties.h"

#include "SceneSerializationBaseExports.h"

namespace mitk
{

class SceneSerializationBase_EXPORT Point3dPropertySerializer : public BasePropertySerializer
{
  public:
    
    mitkClassMacro( Point3dPropertySerializer, BasePropertySerializer );
    itkNewMacro(Self);

    virtual TiXmlElement* Serialize()
    {
      if (const Point3dProperty* prop = dynamic_cast<const Point3dProperty*>(m_Property.GetPointer()))
      {
        TiXmlElement* element = new TiXmlElement("point");
        Point3D point = prop->GetValue();
        element->SetDoubleAttribute("x", point[0]);
        element->SetDoubleAttribute("y", point[1]);
        element->SetDoubleAttribute("z", point[2]);
        return element;
      }
      else return NULL;
    }

  protected:

    Point3dPropertySerializer() {}
    virtual ~Point3dPropertySerializer() {}
};

} // namespace

// important to put this into the GLOBAL namespace (because it starts with 'namespace mitk')
MITK_REGISTER_SERIALIZER(Point3dPropertySerializer);

#endif

