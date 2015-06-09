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

#ifndef mitkVector3DPropertySerializer_h_included
#define mitkVector3DPropertySerializer_h_included

#include "mitkBasePropertySerializer.h"

#include "mitkProperties.h"

namespace mitk
{

class Vector3DPropertySerializer : public BasePropertySerializer
{
  public:

    mitkClassMacro( Vector3DPropertySerializer, BasePropertySerializer );
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    virtual TiXmlElement* Serialize() override
    {
      if (const Vector3DProperty* prop = dynamic_cast<const Vector3DProperty*>(m_Property.GetPointer()))
      {
        auto  element = new TiXmlElement("vector");
        Vector3D point = prop->GetValue();
        element->SetDoubleAttribute("x", point[0]);
        element->SetDoubleAttribute("y", point[1]);
        element->SetDoubleAttribute("z", point[2]);
        return element;
      }
      else return nullptr;
    }

    virtual BaseProperty::Pointer Deserialize(TiXmlElement* element) override
    {
      if (!element) return nullptr;

      Vector3D v;
      if ( element->QueryDoubleAttribute( "x", &v[0] ) != TIXML_SUCCESS ) return nullptr;
      if ( element->QueryDoubleAttribute( "y", &v[1] ) != TIXML_SUCCESS ) return nullptr;
      if ( element->QueryDoubleAttribute( "z", &v[2] ) != TIXML_SUCCESS ) return nullptr;

     return Vector3DProperty::New( v ).GetPointer();
    }

  protected:

    Vector3DPropertySerializer() {}
    virtual ~Vector3DPropertySerializer() {}
};

} // namespace

// important to put this into the GLOBAL namespace (because it starts with 'namespace mitk')
MITK_REGISTER_SERIALIZER(Vector3DPropertySerializer);

#endif

