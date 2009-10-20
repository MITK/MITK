/*=========================================================================
 
Program:   Medical Imaging & Vector3Deraction Toolkit
Module:    $RCSfile: mitkPropertyManager.cpp,v $
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

#ifndef mitkClippingPropertySerializer_h_included
#define mitkClippingPropertySerializer_h_included

#include "mitkBasePropertySerializer.h"

#include "mitkClippingProperty.h"
#include "mitkVector.h"

namespace mitk
{
class SceneSerialization_EXPORT ClippingPropertySerializer : public BasePropertySerializer
{
  public:
    mitkClassMacro( ClippingPropertySerializer, BasePropertySerializer );
    itkNewMacro(Self);

    virtual TiXmlElement* Serialize()
    {
      if (const ClippingProperty* prop = dynamic_cast<const ClippingProperty*>(m_Property.GetPointer()))
      {
        TiXmlElement* element = new TiXmlElement("clipping");
        if (prop->GetClippingEnabled())
          element->SetAttribute("enabled", "true");
        else
          element->SetAttribute("enabled", "false");
        TiXmlElement* originElement = new TiXmlElement("origin");
        const Point3D origin = prop->GetOrigin();
        originElement->SetDoubleAttribute("x", origin[0]);
        originElement->SetDoubleAttribute("y", origin[1]);
        originElement->SetDoubleAttribute("z", origin[2]);
        element->LinkEndChild(originElement);
        TiXmlElement* normalElement = new TiXmlElement("normal");
        const Vector3D normal = prop->GetNormal();
        normalElement->SetDoubleAttribute("x", normal[0]);
        normalElement->SetDoubleAttribute("y", normal[1]);
        normalElement->SetDoubleAttribute("z", normal[2]);
        element->LinkEndChild(normalElement);
        return element;
      }
      else 
        return NULL;
    }
  protected:
    ClippingPropertySerializer() {}
    virtual ~ClippingPropertySerializer() {}
};
} // namespace
// important to put this into the GLOBAL namespace (because it starts with 'namespace mitk')
MITK_REGISTER_SERIALIZER(ClippingPropertySerializer);
#endif
