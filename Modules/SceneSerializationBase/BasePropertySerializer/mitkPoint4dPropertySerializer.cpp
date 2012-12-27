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

#ifndef mitkPoint4dPropertySerializer_h_included
#define mitkPoint4dPropertySerializer_h_included

#include "mitkBasePropertySerializer.h"

#include "mitkProperties.h"

#include "SceneSerializationBaseExports.h"

namespace mitk
{

class SceneSerializationBase_EXPORT Point4dPropertySerializer : public BasePropertySerializer
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
        element->SetDoubleAttribute("x", point[0]);
        element->SetDoubleAttribute("y", point[1]);
        element->SetDoubleAttribute("z", point[2]);
        element->SetDoubleAttribute("t", point[3]);
        return element;
      }
      else return NULL;
    }

    virtual BaseProperty::Pointer Deserialize(TiXmlElement* element)
    {
      if (!element) return NULL;

      Point4D v;
      if ( element->QueryFloatAttribute( "x", &v[0] ) != TIXML_SUCCESS ) return NULL;
      if ( element->QueryFloatAttribute( "y", &v[1] ) != TIXML_SUCCESS ) return NULL;
      if ( element->QueryFloatAttribute( "z", &v[2] ) != TIXML_SUCCESS ) return NULL;
      if ( element->QueryFloatAttribute( "t", &v[3] ) != TIXML_SUCCESS ) return NULL;

     return Point4dProperty::New( v ).GetPointer();
    }

  protected:

    Point4dPropertySerializer() {}
    virtual ~Point4dPropertySerializer() {}
};

} // namespace

// important to put this into the GLOBAL namespace (because it starts with 'namespace mitk')
MITK_REGISTER_SERIALIZER(Point4dPropertySerializer);

#endif

