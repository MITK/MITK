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

#ifndef mitkUShortPropertySerializer_h_included
#define mitkUShortPropertySerializer_h_included

#include "mitkBasePropertySerializer.h"

#include "mitkProperties.h"

namespace mitk
{

class UShortPropertySerializer : public BasePropertySerializer
{
  public:

    mitkClassMacro( UShortPropertySerializer, BasePropertySerializer );
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    virtual TiXmlElement* Serialize() override
    {
      if (const UShortProperty* prop = dynamic_cast<const UShortProperty*>(m_Property.GetPointer()))
      {
        auto  element = new TiXmlElement("unsigned");
        element->SetAttribute("value", static_cast<unsigned short>(prop->GetValue()));
        return element;
      }
      else return nullptr;
    }

    virtual BaseProperty::Pointer Deserialize(TiXmlElement* element) override
    {
      if (!element) return nullptr;

      unsigned int value;
      if ( element->QueryUnsignedAttribute( "value", &value ) == TIXML_SUCCESS )
      {
        return UShortProperty::New(static_cast<unsigned short>(value)).GetPointer();
      }
      else
      {
        return nullptr;
      }
    }

  protected:

    UShortPropertySerializer() {}
    virtual ~UShortPropertySerializer() {}
};

} // namespace

// important to put this into the GLOBAL namespace (because it starts with 'namespace mitk')
MITK_REGISTER_SERIALIZER(UShortPropertySerializer);

#endif

