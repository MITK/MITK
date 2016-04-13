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
#include "mitkStringsToNumbers.h"

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
        auto  element = new TiXmlElement("annotation");
        element->SetAttribute("label", prop->GetLabel());
        Point3D point = prop->GetPosition();
        element->SetAttribute("x", boost::lexical_cast<std::string>(point[0]));
        element->SetAttribute("y", boost::lexical_cast<std::string>(point[1]));
        element->SetAttribute("z", boost::lexical_cast<std::string>(point[2]));
        return element;
      }
      else return nullptr;
    }

    virtual BaseProperty::Pointer Deserialize(TiXmlElement* element) override
    {
      if (!element) return nullptr;
      const char* label( element->Attribute("label") );
      std::string p_string[3];
      if ( element->QueryStringAttribute( "x", &p_string[0] ) != TIXML_SUCCESS )
        return nullptr;
      if ( element->QueryStringAttribute( "y", &p_string[1] ) != TIXML_SUCCESS )
        return nullptr;
      if ( element->QueryStringAttribute( "z", &p_string[2] ) != TIXML_SUCCESS )
        return nullptr;
      Point3D p;
      try
      {
        StringsToNumbers<double>(3, p_string, p);
      }
      catch (boost::bad_lexical_cast& e)
      {
        MITK_ERROR << "Could not parse string as number: " << e.what();
        return nullptr;
      }

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

