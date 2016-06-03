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

#include <mitkPropertyPersistenceInfo.h>
#include <mitkStringProperty.h>
#include <mitkIOMimeTypes.h>

namespace mitk
{
  struct PropertyPersistenceInfo::Impl
  {
    explicit Impl(const std::string& key);
    ~Impl();

    std::string Key;
    DeserializationFunctionType DeSerFnc;
    SerializationFunctionType SerFnc;
    MimeTypeNameType MimeTypeName;
  };

  PropertyPersistenceInfo::Impl::Impl(const std::string& key)
      : Key(key), DeSerFnc(PropertyPersistenceDeserialization::deserializeToStringProperty),
      SerFnc(PropertyPersistenceSerialization::serializeByGetValueAsString)
  {
      MimeTypeName = PropertyPersistenceInfo::ANY_MIMETYPE_NAME();
  }

  PropertyPersistenceInfo::Impl::~Impl()
  {
  }
}

mitk::PropertyPersistenceInfo::PropertyPersistenceInfo(const std::string& key)
  : m_Impl(new Impl(key))
{
}

mitk::PropertyPersistenceInfo::PropertyPersistenceInfo(const std::string& key, const std::string& mimeTypeName)
  : m_Impl(new Impl(key))
{
  m_Impl->MimeTypeName = mimeTypeName;
}

mitk::PropertyPersistenceInfo::~PropertyPersistenceInfo()
{
  delete m_Impl;
}

std::string mitk::PropertyPersistenceInfo::GetKey() const
{
  return m_Impl->Key;
}

void mitk::PropertyPersistenceInfo::SetKey(const std::string& key)
{
  m_Impl->Key = key;
}

const mitk::PropertyPersistenceInfo::MimeTypeNameType mitk::PropertyPersistenceInfo::GetMimeTypeName() const
{
    return m_Impl->MimeTypeName;
};

void mitk::PropertyPersistenceInfo::SetMimeTypeName(const mitk::PropertyPersistenceInfo::MimeTypeNameType& mimeTypeName)
{
    m_Impl->MimeTypeName = mimeTypeName;
};

const mitk::PropertyPersistenceInfo::DeserializationFunctionType mitk::PropertyPersistenceInfo::GetDeserializationFunction() const
{
    return m_Impl->DeSerFnc;
};

void mitk::PropertyPersistenceInfo::SetDeserializationFunction(const mitk::PropertyPersistenceInfo::DeserializationFunctionType& fnc)
{
    m_Impl->DeSerFnc = fnc;
};

const mitk::PropertyPersistenceInfo::SerializationFunctionType mitk::PropertyPersistenceInfo::GetSerializationFunction() const
{
    return m_Impl->SerFnc;
};

void mitk::PropertyPersistenceInfo::SetSerializationFunction(const mitk::PropertyPersistenceInfo::SerializationFunctionType& fnc)
{
    m_Impl->SerFnc = fnc;
};

mitk::PropertyPersistenceInfo::MimeTypeNameType mitk::PropertyPersistenceInfo::ANY_MIMETYPE_NAME()
{
    static std::string name = IOMimeTypes::DEFAULT_BASE_NAME() + ".any_type";
    return name;
};

::std::string mitk::PropertyPersistenceSerialization::serializeByGetValueAsString(const mitk::BaseProperty* prop)
{
    std::string result = "";
    if (prop)
    {
        result = prop->GetValueAsString();
    }
    return result;
}

mitk::BaseProperty::Pointer mitk::PropertyPersistenceDeserialization::deserializeToStringProperty(const std::string& value)
{
    StringProperty::Pointer result = StringProperty::New(value);
    return result.GetPointer();
}
