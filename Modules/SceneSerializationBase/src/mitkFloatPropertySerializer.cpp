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

#ifndef mitkFloatPropertySerializer_h_included
#define mitkFloatPropertySerializer_h_included

#include "mitkBasePropertySerializer.h"

#include "mitkProperties.h"

namespace mitk
{

class FloatPropertySerializer : public BasePropertySerializer
{
  public:

    mitkClassMacro( FloatPropertySerializer, BasePropertySerializer );
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    virtual TiXmlElement* Serialize() override
    {
      if (const FloatProperty* prop = dynamic_cast<const FloatProperty*>(m_Property.GetPointer()))
      {
        auto  element = new TiXmlElement("float");
        element->SetDoubleAttribute("value", static_cast<double>(prop->GetValue()));
        return element;
      }
      else return nullptr;
    }

    virtual BaseProperty::Pointer Deserialize(TiXmlElement* element) override
    {
      if (!element) return nullptr;

      float f;
      if ( element->QueryFloatAttribute( "value", &f ) == TIXML_SUCCESS )
      {
        return FloatProperty::New(f).GetPointer();
      }
      else
      {
        return nullptr;
      }
    }

  protected:

    FloatPropertySerializer() {}
    virtual ~FloatPropertySerializer() {}
};

} // namespace

// important to put this into the GLOBAL namespace (because it starts with 'namespace mitk')
MITK_REGISTER_SERIALIZER(FloatPropertySerializer);

#endif

