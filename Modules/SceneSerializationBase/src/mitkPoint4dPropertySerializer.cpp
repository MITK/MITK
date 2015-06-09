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

namespace mitk
{

class Point4dPropertySerializer : public BasePropertySerializer
{
  public:

    mitkClassMacro( Point4dPropertySerializer, BasePropertySerializer );
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    virtual TiXmlElement* Serialize() override
    {
      if (const Point4dProperty* prop = dynamic_cast<const Point4dProperty*>(m_Property.GetPointer()))
      {
        auto  element = new TiXmlElement("point");
        Point4D point = prop->GetValue();
        element->SetDoubleAttribute("x", point[0]);
        element->SetDoubleAttribute("y", point[1]);
        element->SetDoubleAttribute("z", point[2]);
        element->SetDoubleAttribute("t", point[3]);
        return element;
      }
      else return nullptr;
    }

    virtual BaseProperty::Pointer Deserialize(TiXmlElement* element) override
    {
      if (!element) return nullptr;

      Point4D v;
      if ( element->QueryDoubleAttribute( "x", &v[0] ) != TIXML_SUCCESS ) return nullptr;
      if ( element->QueryDoubleAttribute( "y", &v[1] ) != TIXML_SUCCESS ) return nullptr;
      if ( element->QueryDoubleAttribute( "z", &v[2] ) != TIXML_SUCCESS ) return nullptr;
      if ( element->QueryDoubleAttribute( "t", &v[3] ) != TIXML_SUCCESS ) return nullptr;

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

