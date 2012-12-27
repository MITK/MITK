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

#include "SceneSerializationBaseExports.h"

namespace mitk
{

class SceneSerializationBase_EXPORT Vector3DPropertySerializer : public BasePropertySerializer
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
        element->SetDoubleAttribute("x", point[0]);
        element->SetDoubleAttribute("y", point[1]);
        element->SetDoubleAttribute("z", point[2]);
        return element;
      }
      else return NULL;
    }

    virtual BaseProperty::Pointer Deserialize(TiXmlElement* element)
    {
      if (!element) return NULL;

      Vector3D v;
      if ( element->QueryFloatAttribute( "x", &v[0] ) != TIXML_SUCCESS ) return NULL;
      if ( element->QueryFloatAttribute( "y", &v[1] ) != TIXML_SUCCESS ) return NULL;
      if ( element->QueryFloatAttribute( "z", &v[2] ) != TIXML_SUCCESS ) return NULL;

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

