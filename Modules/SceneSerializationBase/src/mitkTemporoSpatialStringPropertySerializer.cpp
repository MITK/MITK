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

#ifndef mitkTemporoSpatialStringPropertySerializer_h_included
#define mitkTemporoSpatialStringPropertySerializer_h_included

#include "mitkBasePropertySerializer.h"

#include "mitkTemporoSpatialStringProperty.h"

namespace mitk
{
  class TemporoSpatialStringPropertySerializer : public BasePropertySerializer
  {
  public:
    mitkClassMacro(TemporoSpatialStringPropertySerializer, BasePropertySerializer);
    itkFactorylessNewMacro(Self) itkCloneMacro(Self)

      virtual TiXmlElement *Serialize() override
    {
      if (const TemporoSpatialStringProperty *prop =
            dynamic_cast<const TemporoSpatialStringProperty *>(m_Property.GetPointer()))
      {
        auto element = new TiXmlElement("temporo_spatial_string");
        std::string content = PropertyPersistenceSerialization::serializeTemporoSpatialStringPropertyToJSON(prop);
        TiXmlText text(content.c_str());
        element->InsertEndChild(text);
        return element;
      }
      else
        return nullptr;
    }

    virtual BaseProperty::Pointer Deserialize(TiXmlElement *element) override
    {
      if (!element)
        return nullptr;
      const char *s(element->GetText());
      std::string content = s ? s : "";
      return PropertyPersistenceDeserialization::deserializeJSONToTemporoSpatialStringProperty(content);
    }

  protected:
    TemporoSpatialStringPropertySerializer() {}
    virtual ~TemporoSpatialStringPropertySerializer() {}
  };

} // namespace

// important to put this into the GLOBAL namespace (because it starts with 'namespace mitk')
MITK_REGISTER_SERIALIZER(TemporoSpatialStringPropertySerializer);

#endif
