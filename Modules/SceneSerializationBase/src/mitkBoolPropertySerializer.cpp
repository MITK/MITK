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

#ifndef mitkBoolPropertySerializer_h_included
#define mitkBoolPropertySerializer_h_included

#include "mitkBasePropertySerializer.h"

#include "mitkProperties.h"

namespace mitk
{

class BoolPropertySerializer : public BasePropertySerializer
{
  public:

    mitkClassMacro( BoolPropertySerializer, BasePropertySerializer );
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    virtual TiXmlElement* Serialize() override
    {
      if (const BoolProperty* prop = dynamic_cast<const BoolProperty*>(m_Property.GetPointer()))
      {
        auto  element = new TiXmlElement("bool");
        if (prop->GetValue() == true)
        {
          element->SetAttribute("value", "true");
        }
        else
        {
          element->SetAttribute("value", "false");
        }
        return element;
      }
      else return nullptr;
    }

    virtual BaseProperty::Pointer Deserialize(TiXmlElement* element) override
    {
      if (!element) return nullptr;
      return BoolProperty::New( std::string(element->Attribute("value")) == "true" ).GetPointer();
    }

  protected:

    BoolPropertySerializer() {}
    virtual ~BoolPropertySerializer() {}
};

} // namespace

// important to put this into the GLOBAL namespace (because it starts with 'namespace mitk')
MITK_REGISTER_SERIALIZER(BoolPropertySerializer);

#endif

