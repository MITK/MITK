/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkBoolPropertySerializer_h_included
#define mitkBoolPropertySerializer_h_included

#include "mitkBasePropertySerializer.h"

#include "mitkProperties.h"

namespace mitk
{
  class BoolPropertySerializer : public BasePropertySerializer
  {
  public:
    mitkClassMacro(BoolPropertySerializer, BasePropertySerializer);
    itkFactorylessNewMacro(Self) itkCloneMacro(Self)

      TiXmlElement *Serialize() override
    {
      if (const BoolProperty *prop = dynamic_cast<const BoolProperty *>(m_Property.GetPointer()))
      {
        auto element = new TiXmlElement("bool");
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
      else
        return nullptr;
    }

    BaseProperty::Pointer Deserialize(TiXmlElement *element) override
    {
      if (!element)
        return nullptr;
      return BoolProperty::New(std::string(element->Attribute("value")) == "true").GetPointer();
    }

  protected:
    BoolPropertySerializer() {}
    ~BoolPropertySerializer() override {}
  };

} // namespace

// important to put this into the GLOBAL namespace (because it starts with 'namespace mitk')
MITK_REGISTER_SERIALIZER(BoolPropertySerializer);

#endif
