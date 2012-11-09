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

#include "SceneSerializationBaseExports.h"

namespace mitk
{

class SceneSerializationBase_EXPORT FloatPropertySerializer : public BasePropertySerializer
{
  public:

    mitkClassMacro( FloatPropertySerializer, BasePropertySerializer );
    itkNewMacro(Self);

    virtual TiXmlElement* Serialize()
    {
      if (const FloatProperty* prop = dynamic_cast<const FloatProperty*>(m_Property.GetPointer()))
      {
        TiXmlElement* element = new TiXmlElement("float");
        element->SetDoubleAttribute("value", static_cast<double>(prop->GetValue()));
        return element;
      }
      else return NULL;
    }

    virtual BaseProperty::Pointer Deserialize(TiXmlElement* element)
    {
      if (!element) return NULL;

      float f;
      if ( element->QueryFloatAttribute( "value", &f ) == TIXML_SUCCESS )
      {
        return FloatProperty::New(f).GetPointer();
      }
      else
      {
        return NULL;
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

