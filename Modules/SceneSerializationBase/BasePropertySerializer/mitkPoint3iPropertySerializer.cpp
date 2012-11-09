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

#ifndef mitkPoint3iPropertySerializer_h_included
#define mitkPoint3iPropertySerializer_h_included

#include "mitkBasePropertySerializer.h"
#include "mitkProperties.h"

#include "SceneSerializationBaseExports.h"

namespace mitk
{
class SceneSerializationBase_EXPORT Point3iPropertySerializer : public BasePropertySerializer
{
  public:
    mitkClassMacro( Point3iPropertySerializer, BasePropertySerializer );
    itkNewMacro(Self);

    virtual TiXmlElement* Serialize()
    {
      if (const Point3iProperty* prop = dynamic_cast<const Point3iProperty*>(m_Property.GetPointer()))
      {
        TiXmlElement* element = new TiXmlElement("point");
        Point3I point = prop->GetValue();
        element->SetAttribute("x", point[0]);
        element->SetAttribute("y", point[1]);
        element->SetAttribute("z", point[2]);
        return element;
      }
      else return NULL;
    }

    virtual BaseProperty::Pointer Deserialize(TiXmlElement* element)
    {
      if (!element) return NULL;

      Point3I v;
      if ( element->QueryIntAttribute( "x", &v[0] ) != TIXML_SUCCESS ) return NULL;
      if ( element->QueryIntAttribute( "y", &v[1] ) != TIXML_SUCCESS ) return NULL;
      if ( element->QueryIntAttribute( "z", &v[2] ) != TIXML_SUCCESS ) return NULL;
     return Point3iProperty::New( v ).GetPointer();
    }
  protected:
    Point3iPropertySerializer() {}
    virtual ~Point3iPropertySerializer() {}
};
} // namespace
// important to put this into the GLOBAL namespace (because it starts with 'namespace mitk')
MITK_REGISTER_SERIALIZER(Point3iPropertySerializer);
#endif
