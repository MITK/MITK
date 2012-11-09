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


#include "mitkSerializerMacros.h"
#include "mitkPropertyListDeserializerV1.h"
#include "mitkBasePropertySerializer.h"
#include <tinyxml.h>

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

  TiXmlDocument document( m_Filename );
  if (!document.LoadFile())
  {
    MITK_ERROR << "Could not open/read/parse " << m_Filename << "\nTinyXML reports: " << document.ErrorDesc() << std::endl;
    return false;
  }

  for( TiXmlElement* propertyElement = document.FirstChildElement("property"); propertyElement != NULL; propertyElement = propertyElement->NextSiblingElement("property") )
  {
    const char* keya = propertyElement->Attribute("key");
    std::string key( keya ? keya : "");

    const char* typea = propertyElement->Attribute("type");
    std::string type( typea ? typea : "");

    // hand propertyElement to specific reader
    std::stringstream propertyDeserializerClassName;
    propertyDeserializerClassName << type << "Serializer";

    std::list<itk::LightObject::Pointer> readers = itk::ObjectFactoryBase::CreateAllInstance(propertyDeserializerClassName.str().c_str());
    if (readers.size() < 1)
    {
      MITK_ERROR << "No property reader found for " << type;
      error = true;
    }
    if (readers.size() > 1)
    {
      MITK_WARN << "Multiple property readers found for " << type << ". Using arbitrary first one.";
    }

    for ( std::list<itk::LightObject::Pointer>::iterator iter = readers.begin();
          iter != readers.end();
          ++iter )
    {
      if (BasePropertySerializer* reader = dynamic_cast<BasePropertySerializer*>( iter->GetPointer() ) )
      {
        BaseProperty::Pointer property = reader->Deserialize( propertyElement->FirstChildElement() );
        if (property.IsNotNull())
        {
          m_PropertyList->ReplaceProperty(key, property);
        }
        else
        {
          MITK_ERROR << "There were errors while loding property '" << key << "' of type " << type << ". Your data may be corrupted";
          error = true;
        }
        break;
      }
    }
  }
  return !error;
}
