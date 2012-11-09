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

#ifndef mitkDoublePropertySerializer_h_included
#define mitkDoublePropertySerializer_h_included

#include "mitkBasePropertySerializer.h"

#include "mitkProperties.h"

#include "SceneSerializationBaseExports.h"

namespace mitk
{

class SceneSerializationBase_EXPORT DoublePropertySerializer : public BasePropertySerializer
{
  public:

    mitkClassMacro( DoublePropertySerializer, BasePropertySerializer );
    itkNewMacro(Self);

    virtual TiXmlElement* Serialize()
    {
      if (const DoubleProperty* prop = dynamic_cast<const DoubleProperty*>(m_Property.GetPointer()))
      {
        TiXmlElement* element = new TiXmlElement("double");
        element->SetDoubleAttribute("value", prop->GetValue());
        return element;
      }
      else return NULL;
    }

    virtual BaseProperty::Pointer Deserialize(TiXmlElement* element)
    {
      if (!element) return NULL;

      double d;
      if ( element->QueryDoubleAttribute( "value", &d ) == TIXML_SUCCESS )
      {
        return DoubleProperty::New(d).GetPointer();
      }
      else
      {
        return NULL;
      }
    }

  protected:

    DoublePropertySerializer() {}
    virtual ~DoublePropertySerializer() {}
};

} // namespace

// important to put this into the GLOBAL namespace (because it starts with 'namespace mitk')
MITK_REGISTER_SERIALIZER(DoublePropertySerializer);

#endif

