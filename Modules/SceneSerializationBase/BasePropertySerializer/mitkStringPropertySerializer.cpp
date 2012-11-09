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

#ifndef mitkStringPropertySerializer_h_included
#define mitkStringPropertySerializer_h_included

#include "mitkBasePropertySerializer.h"

#include "mitkStringProperty.h"

#include "SceneSerializationBaseExports.h"

namespace mitk
{

class SceneSerializationBase_EXPORT StringPropertySerializer : public BasePropertySerializer
{
  public:

    mitkClassMacro( StringPropertySerializer, BasePropertySerializer );
    itkNewMacro(Self);

    virtual TiXmlElement* Serialize()
    {
      if (const StringProperty* prop = dynamic_cast<const StringProperty*>(m_Property.GetPointer()))
      {
        TiXmlElement* element = new TiXmlElement("string");
        element->SetAttribute("value", prop->GetValue());
        return element;
      }
      else return NULL;
    }

    virtual BaseProperty::Pointer Deserialize(TiXmlElement* element)
    {
      if (!element) return NULL;
      const char* s( element->Attribute("value") );
      return StringProperty::New( std::string(s?s:"") ).GetPointer();
    }

  protected:

    StringPropertySerializer() {}
    virtual ~StringPropertySerializer() {}
};

} // namespace

// important to put this into the GLOBAL namespace (because it starts with 'namespace mitk')
MITK_REGISTER_SERIALIZER(StringPropertySerializer);

#endif

