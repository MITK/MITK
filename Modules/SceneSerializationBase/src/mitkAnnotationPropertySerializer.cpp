/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkAnnotationPropertySerializer_h_included
#define mitkAnnotationPropertySerializer_h_included

#include "mitkBasePropertySerializer.h"

#include "mitkAnnotationProperty.h"
#include "mitkStringsToNumbers.h"

#include <array>

#include <tinyxml2.h>

namespace mitk
{
  class AnnotationPropertySerializer : public BasePropertySerializer
  {
  public:
    mitkClassMacro(AnnotationPropertySerializer, BasePropertySerializer)
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    tinyxml2::XMLElement *Serialize(tinyxml2::XMLDocument& doc) override
    {
      if (const AnnotationProperty *prop = dynamic_cast<const AnnotationProperty *>(m_Property.GetPointer()))
      {
        auto *element = doc.NewElement("annotation");
        element->SetAttribute("label", prop->GetLabel());
        Point3D point = prop->GetPosition();
        element->SetAttribute("x", boost::lexical_cast<std::string>(point[0]).c_str());
        element->SetAttribute("y", boost::lexical_cast<std::string>(point[1]).c_str());
        element->SetAttribute("z", boost::lexical_cast<std::string>(point[2]).c_str());
        return element;
      }
      else
        return nullptr;
    }

    BaseProperty::Pointer Deserialize(const tinyxml2::XMLElement *element) override
    {
      if (!element)
        return nullptr;
      const char *label(element->Attribute("label"));
      std::array<const char*, 3> p_string = {
        element->Attribute("x"),
        element->Attribute("y"),
        element->Attribute("z")
      };
      if (nullptr == p_string[0] || nullptr == p_string[1] || nullptr == p_string[2])
        return nullptr;
      Point3D p;
      try
      {
        StringsToNumbers<double>(3, p_string, p);
      }
      catch (boost::bad_lexical_cast &e)
      {
        MITK_ERROR << "Could not parse string as number: " << e.what();
        return nullptr;
      }

      return AnnotationProperty::New(label, p).GetPointer();
    }

  protected:
    AnnotationPropertySerializer() {}
    ~AnnotationPropertySerializer() override {}
  };

} // namespace

// important to put this into the GLOBAL namespace (because it starts with 'namespace mitk')
MITK_REGISTER_SERIALIZER(AnnotationPropertySerializer);

#endif
