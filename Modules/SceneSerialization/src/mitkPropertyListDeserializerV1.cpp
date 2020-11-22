/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkPropertyListDeserializerV1.h"
#include "mitkBasePropertySerializer.h"
#include "mitkSerializerMacros.h"
#include <tinyxml2.h>

MITK_REGISTER_SERIALIZER(PropertyListDeserializerV1)

mitk::PropertyListDeserializerV1::PropertyListDeserializerV1()
{
}

mitk::PropertyListDeserializerV1::~PropertyListDeserializerV1()
{
}

bool mitk::PropertyListDeserializerV1::Deserialize()
{
  bool error(false);

  m_PropertyList = PropertyList::New();

  tinyxml2::XMLDocument document;
  if (tinyxml2::XML_SUCCESS != document.LoadFile(m_Filename.c_str()))
  {
    MITK_ERROR << "Could not open/read/parse " << m_Filename << "\nTinyXML reports: " << document.ErrorStr()
               << std::endl;
    return false;
  }

  for (auto *propertyElement = document.FirstChildElement("property"); propertyElement != nullptr;
       propertyElement = propertyElement->NextSiblingElement("property"))
  {
    const char *keya = propertyElement->Attribute("key");
    std::string key(keya ? keya : "");

    const char *typea = propertyElement->Attribute("type");
    std::string type(typea ? typea : "");

    // hand propertyElement to specific reader
    std::stringstream propertyDeserializerClassName;
    propertyDeserializerClassName << type << "Serializer";

    std::list<itk::LightObject::Pointer> readers =
      itk::ObjectFactoryBase::CreateAllInstance(propertyDeserializerClassName.str().c_str());
    if (readers.size() < 1)
    {
      MITK_ERROR << "No property reader found for " << type;
      error = true;
    }
    if (readers.size() > 1)
    {
      MITK_WARN << "Multiple property readers found for " << type << ". Using arbitrary first one.";
    }

    for (auto iter = readers.begin(); iter != readers.end(); ++iter)
    {
      if (auto *reader = dynamic_cast<BasePropertySerializer *>(iter->GetPointer()))
      {
        BaseProperty::Pointer property = reader->Deserialize(propertyElement->FirstChildElement());
        if (property.IsNotNull())
        {
          m_PropertyList->ReplaceProperty(key, property);
        }
        else
        {
          MITK_ERROR << "There were errors while loading property '" << key << "' of type " << type
                     << ". Your data may be corrupted";
          error = true;
        }
        break;
      }
    }
  }
  return !error;
}
