/*=========================================================================
 
Program:   Medical Imaging & Stringeraction Toolkit
Language:  C++
Date:      $Date: 2009-07-07 15:56:37 +0200 (Di, 07. Jul 2009) $
Version:   $Revision: 1.12 $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#ifndef mitkAnnotationPropertySerializer_h_included
#define mitkAnnotationPropertySerializer_h_included

#include "mitkBasePropertySerializer.h"

#include "mitkAnnotationProperty.h"

namespace mitk
{

class SceneSerialization_EXPORT AnnotationPropertySerializer : public BasePropertySerializer
{
  public:
    
    mitkClassMacro( AnnotationPropertySerializer, BasePropertySerializer );
    itkNewMacro(Self);

    virtual TiXmlElement* Serialize()
    {
      if (const AnnotationProperty* prop = dynamic_cast<const AnnotationProperty*>(m_Property.GetPointer()))
      {
        TiXmlElement* element = new TiXmlElement("annotation");
        element->SetAttribute("label", prop->GetLabel());
        Point3D point = prop->GetPosition();
        element->SetDoubleAttribute("x", point[0]);
        element->SetDoubleAttribute("y", point[1]);
        element->SetDoubleAttribute("z", point[2]);
        return element;
      }
      else return NULL;
    }

  protected:

    AnnotationPropertySerializer() {}
    virtual ~AnnotationPropertySerializer() {}
};

} // namespace

// important to put this into the GLOBAL namespace (because it starts with 'namespace mitk')
MITK_REGISTER_SERIALIZER(AnnotationPropertySerializer);

#endif

