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

#ifndef mitkUIntPropertySerializer_h_included
#define mitkUIntPropertySerializer_h_included

#include "mitkBasePropertySerializer.h"

#include "mitkProperties.h"

namespace mitk
{

class UIntPropertySerializer : public BasePropertySerializer
{
  public:

    mitkClassMacro( UIntPropertySerializer, BasePropertySerializer );
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    virtual TiXmlElement* Serialize() override
    {
      if (const UIntProperty* prop = dynamic_cast<const UIntProperty*>(m_Property.GetPointer()))
      {
        TiXmlElement* element = new TiXmlElement("unsigned");
        element->SetAttribute("value", static_cast<unsigned int>(prop->GetValue()));
        return element;
      }
      else return NULL;
    }

    virtual BaseProperty::Pointer Deserialize(TiXmlElement* element) override
    {
      if (!element) return NULL;

      unsigned int integer;
      if ( element->QueryUnsignedAttribute( "value", &integer ) == TIXML_SUCCESS )
      {
        return UIntProperty::New(integer).GetPointer();
      }
      else
      {
        return NULL;
      }
    }

  protected:

    UIntPropertySerializer() {}
    virtual ~UIntPropertySerializer() {}
};

} // namespace

// important to put this into the GLOBAL namespace (because it starts with 'namespace mitk')
MITK_REGISTER_SERIALIZER(UIntPropertySerializer);

#endif

