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

#include <cassert>
#include <regex>

#include <mitkIOMimeTypes.h>
#include <mitkPropertyPersistenceInfo.h>
#include <mitkStringProperty.h>

namespace mitk
{
  struct PropertyPersistenceInfo::Impl
  {
    Impl();
    ~Impl();

    std::string Name;
    std::string Key;
    bool IsRegEx;
    std::string NameTemplate;
    std::string KeyTemplate;
    DeserializationFunctionType DeSerFnc;
    SerializationFunctionType SerFnc;
    MimeTypeNameType MimeTypeName;
  };

  PropertyPersistenceInfo::Impl::Impl()
    : Name(""),
      Key(""),
      IsRegEx(false),
      DeSerFnc(PropertyPersistenceDeserialization::deserializeToStringProperty),
      SerFnc(PropertyPersistenceSerialization::serializeByGetValueAsString),
      MimeTypeName(PropertyPersistenceInfo::ANY_MIMETYPE_NAME())
  {
  }

  PropertyPersistenceInfo::Impl::~Impl() {}
}

mitk::PropertyPersistenceInfo::PropertyPersistenceInfo(const std::string &name) : m_Impl(new Impl())
{
  m_Impl->Name = name;
  m_Impl->Key = name;
}

mitk::PropertyPersistenceInfo::PropertyPersistenceInfo(const std::string &name, const std::string &mimeTypeName)
  : m_Impl(new Impl())
{
  m_Impl->Name = name;
  m_Impl->Key = name;
  m_Impl->MimeTypeName = mimeTypeName;
}

mitk::PropertyPersistenceInfo::~PropertyPersistenceInfo()
{
  delete m_Impl;
}

std::string mitk::PropertyPersistenceInfo::GetName() const
{
  return m_Impl->Name;
}

std::string mitk::PropertyPersistenceInfo::GetKey() const
{
  return m_Impl->Key;
}

void mitk::PropertyPersistenceInfo::SetName(const std::string &name)
{
  m_Impl->Name = name;
  m_Impl->Key = name;
  m_Impl->IsRegEx = false;
  m_Impl->NameTemplate.clear();
  m_Impl->KeyTemplate.clear();
}

void mitk::PropertyPersistenceInfo::SetNameAndKey(const std::string &name, const std::string &key)
{
  m_Impl->Name = name;
  m_Impl->Key = key;
  m_Impl->IsRegEx = false;
  m_Impl->NameTemplate.clear();
  m_Impl->KeyTemplate.clear();
}

void mitk::PropertyPersistenceInfo::UseRegEx(const std::string &nameRegEx, const std::string &nameTemplate)
{
  std::regex checker(nameRegEx); // no exception => valid we can change the info
  m_Impl->Name = nameRegEx;
  m_Impl->Key = nameRegEx;
  m_Impl->IsRegEx = true;
  m_Impl->NameTemplate = nameTemplate;
  m_Impl->KeyTemplate = nameTemplate;
}

void mitk::PropertyPersistenceInfo::UseRegEx(const std::string &nameRegEx,
                                             const std::string &nameTemplate,
                                             const std::string &keyRegEx,
                                             const std::string keyTemplate)
{
  std::regex nameChecker(nameRegEx); // no exception => valid we can change the info
  std::regex keyChecker(keyRegEx);   // no exception => valid we can change the info
  m_Impl->Name = nameRegEx;
  m_Impl->Key = keyRegEx;
  m_Impl->IsRegEx = true;
  m_Impl->NameTemplate = nameTemplate;
  m_Impl->KeyTemplate = keyTemplate;
}

bool mitk::PropertyPersistenceInfo::IsRegEx() const
{
  return m_Impl->IsRegEx;
};

const std::string &mitk::PropertyPersistenceInfo::GetKeyTemplate() const
{
  return m_Impl->KeyTemplate;
}

const std::string &mitk::PropertyPersistenceInfo::GetNameTemplate() const
{
  return m_Impl->NameTemplate;
}

const mitk::PropertyPersistenceInfo::MimeTypeNameType &mitk::PropertyPersistenceInfo::GetMimeTypeName() const
{
  return m_Impl->MimeTypeName;
};

void mitk::PropertyPersistenceInfo::SetMimeTypeName(const mitk::PropertyPersistenceInfo::MimeTypeNameType &mimeTypeName)
{
  m_Impl->MimeTypeName = mimeTypeName;
};

const mitk::PropertyPersistenceInfo::DeserializationFunctionType
  mitk::PropertyPersistenceInfo::GetDeserializationFunction() const
{
  return m_Impl->DeSerFnc;
};

void mitk::PropertyPersistenceInfo::SetDeserializationFunction(
  const mitk::PropertyPersistenceInfo::DeserializationFunctionType &fnc)
{
  m_Impl->DeSerFnc = fnc;
};

const mitk::PropertyPersistenceInfo::SerializationFunctionType mitk::PropertyPersistenceInfo::GetSerializationFunction()
  const
{
  return m_Impl->SerFnc;
};

void mitk::PropertyPersistenceInfo::SetSerializationFunction(
  const mitk::PropertyPersistenceInfo::SerializationFunctionType &fnc)
{
  m_Impl->SerFnc = fnc;
};

std::string GenerateFromTemplate(const std::string &sourceStr,
                                 const std::string &templateStr,
                                 const std::string &regexStr)
{
  std::smatch sm;
  std::regex ex(regexStr);
  std::regex_match(sourceStr, sm, ex);

  std::string result = templateStr;

  int groupID = 0;
  for (const auto &match : sm)
  {
    if (groupID)
    {
      std::ostringstream stream;
      stream << "(\\$" << groupID << ")";
      std::regex rex(stream.str());
      result = std::regex_replace(result, rex, match.str());
    }
    ++groupID;
  }

  return result;
};

mitk::PropertyPersistenceInfo::Pointer mitk::PropertyPersistenceInfo::UnRegExByName(
  const std::string &propertyName) const
{
  PropertyPersistenceInfo::Pointer resultInfo = PropertyPersistenceInfo::New();
  *(resultInfo->m_Impl) = *(this->m_Impl);

  if (this->IsRegEx())
  {
    std::string newKey = GenerateFromTemplate(propertyName, this->GetKeyTemplate(), this->GetName());
    resultInfo->SetNameAndKey(propertyName, newKey);
  }

  return resultInfo;
};

mitk::PropertyPersistenceInfo::Pointer mitk::PropertyPersistenceInfo::UnRegExByKey(const std::string &key) const
{
  PropertyPersistenceInfo::Pointer resultInfo = PropertyPersistenceInfo::New();
  *(resultInfo->m_Impl) = *(this->m_Impl);

  if (this->IsRegEx())
  {
    std::string newName = GenerateFromTemplate(key, this->GetNameTemplate(), this->GetKey());
    resultInfo->SetNameAndKey(newName, key);
  }

  return resultInfo;
};

mitk::PropertyPersistenceInfo::MimeTypeNameType mitk::PropertyPersistenceInfo::ANY_MIMETYPE_NAME()
{
  static std::string name = IOMimeTypes::DEFAULT_BASE_NAME() + ".any_type";
  return name;
};

void mitk::PropertyPersistenceInfo::PrintSelf(std::ostream &os, itk::Indent indent) const
{
  this->Superclass::PrintSelf(os, indent);

  os << indent << "Name:         " << this->m_Impl->Name << std::endl;
  os << indent << "Key:          " << this->m_Impl->Key << std::endl;
  os << indent << "IsRegEx:      " << this->m_Impl->IsRegEx << std::endl;
  os << indent << "NameTemplate: " << this->m_Impl->NameTemplate << std::endl;
  os << indent << "KeyTemplate:  " << this->m_Impl->KeyTemplate << std::endl;
  os << indent << "MimeTypeName: " << this->m_Impl->MimeTypeName << std::endl;
};

std::ostream &mitk::operator<<(std::ostream &os, const PropertyPersistenceInfo &info)
{
  info.Print(os);
  return os;
}

::std::string mitk::PropertyPersistenceSerialization::serializeByGetValueAsString(const mitk::BaseProperty *prop)
{
  std::string result = "";
  if (prop)
  {
    result = prop->GetValueAsString();
  }
  return result;
}

mitk::BaseProperty::Pointer mitk::PropertyPersistenceDeserialization::deserializeToStringProperty(
  const std::string &value)
{
  StringProperty::Pointer result = StringProperty::New(value);
  return result.GetPointer();
}
