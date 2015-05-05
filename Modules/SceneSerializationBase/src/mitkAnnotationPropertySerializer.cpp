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

#ifndef mitkAnnotationPropertySerializer_h_included
#define mitkAnnotationPropertySerializer_h_included

#include "mitkBasePropertySerializer.h"

#include "mitkAnnotationProperty.h"

namespace mitk
{

class AnnotationPropertySerializer : public BasePropertySerializer
{
  public:

    mitkClassMacro( AnnotationPropertySerializer, BasePropertySerializer );
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    virtual TiXmlElement* Serialize() override
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

    virtual BaseProperty::Pointer Deserialize(TiXmlElement* element) override
    {
      if (!element) return NULL;
      const char* label( element->Attribute("label") );
      Point3D p;
      if ( element->QueryDoubleAttribute( "x", &p[0] ) != TIXML_SUCCESS )
        return NULL;
      if ( element->QueryDoubleAttribute( "y", &p[1] ) != TIXML_SUCCESS )
        return NULL;
      if ( element->QueryDoubleAttribute( "z", &p[2] ) != TIXML_SUCCESS )
        return NULL;
      return AnnotationProperty::New(label, p).GetPointer();
    }

  protected:

    AnnotationPropertySerializer() {}
    virtual ~AnnotationPropertySerializer() {}
};

} // namespace

// important to put this into the GLOBAL namespace (because it starts with 'namespace mitk')
MITK_REGISTER_SERIALIZER(AnnotationPropertySerializer);

#endif

