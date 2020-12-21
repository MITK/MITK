/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPoint3iPropertySerializer_h_included
#define mitkPoint3iPropertySerializer_h_included

#include "mitkBasePropertySerializer.h"
#include "mitkProperties.h"
#include <tinyxml2.h>

namespace mitk
{
  class Point3iPropertySerializer : public BasePropertySerializer
  {
  public:
    mitkClassMacro(Point3iPropertySerializer, BasePropertySerializer)
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    tinyxml2::XMLElement* Serialize(tinyxml2::XMLDocument& doc) override
    {
      if (const Point3iProperty *prop = dynamic_cast<const Point3iProperty *>(m_Property.GetPointer()))
      {
        auto *element = doc.NewElement("point");
        Point3I point = prop->GetValue();
        element->SetAttribute("x", point[0]);
        element->SetAttribute("y", point[1]);
        element->SetAttribute("z", point[2]);
        return element;
      }
      else
        return nullptr;
    }

    BaseProperty::Pointer Deserialize(const tinyxml2::XMLElement *element) override
    {
      if (!element)
        return nullptr;

      Point3I v;
      if (element->QueryIntAttribute("x", &v[0]) != tinyxml2::XML_SUCCESS)
        return nullptr;
      if (element->QueryIntAttribute("y", &v[1]) != tinyxml2::XML_SUCCESS)
        return nullptr;
      if (element->QueryIntAttribute("z", &v[2]) != tinyxml2::XML_SUCCESS)
        return nullptr;
      return Point3iProperty::New(v).GetPointer();
    }

  protected:
    Point3iPropertySerializer() {}
    ~Point3iPropertySerializer() override {}
  };
} // namespace
// important to put this into the GLOBAL namespace (because it starts with 'namespace mitk')
MITK_REGISTER_SERIALIZER(Point3iPropertySerializer);
#endif
