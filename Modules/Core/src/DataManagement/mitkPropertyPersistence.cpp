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

#include <mitkPropertyPersistence.h>
#include <algorithm>
#include <utility>

mitk::PropertyPersistence::PropertyPersistence()
{
}

mitk::PropertyPersistence::~PropertyPersistence()
{
}

bool mitk::PropertyPersistence::AddInfo(const std::string& propertyName, PropertyPersistenceInfo::Pointer info, bool overwrite)
{
  if (propertyName.empty())
  {
    return false;
  }

  if (info.IsNull())
  {
    return false;
  }

  PropertyPersistenceInfo::MimeTypeNameType mime = info->GetMimeTypeName();

  auto infoRange = m_Infos.equal_range(propertyName);

  auto predicate = [mime](const std::pair<const std::string, PropertyPersistenceInfo::Pointer>& x){return x.second.IsNotNull() && x.second->GetMimeTypeName() == mime; };

  auto finding = std::find_if(infoRange.first, infoRange.second, predicate);

  bool exists = finding != infoRange.second;
  bool result = false;

  if (!exists || overwrite)
  {
    if (exists && overwrite)
    {
      m_Infos.erase(finding);
    }
    result = true;
    m_Infos.insert(std::make_pair(propertyName, info));
  }

  return result;
}

mitk::PropertyPersistence::InfoMapType mitk::PropertyPersistence::GetInfos(const std::string& propertyName)
{
  auto infoRange = m_Infos.equal_range(propertyName);

  InfoMapType result;

  for (auto pos = infoRange.first; pos != infoRange.second; ++pos)
  {
    result.insert(std::make_pair(pos->first, pos->second));
  }

  return result;
}

mitk::PropertyPersistence::InfoMapType mitk::PropertyPersistence::GetInfosByKey(const std::string& persistenceKey)
{
  InfoMapType result;

  for (auto pos : m_Infos)
  {
    if (pos.second.IsNotNull() && pos.second->GetKey() == persistenceKey)
    {
      result.insert(std::make_pair(pos.first, pos.second));
    }
  }

  return result;
}

mitk::PropertyPersistenceInfo::Pointer mitk::PropertyPersistence::GetInfo(const std::string& propertyName, const MimeTypeNameType& mime, bool allowWildCard)
{
  auto infoRange = m_Infos.equal_range(propertyName);

  auto predicate = [mime](const std::pair<const std::string, PropertyPersistenceInfo::Pointer>& x){return x.second.IsNotNull() && x.second->GetMimeTypeName() == mime; };

  auto finding = std::find_if(infoRange.first, infoRange.second, predicate);

  mitk::PropertyPersistenceInfo::Pointer result = nullptr;

  if (finding == infoRange.second && allowWildCard)
  {
    auto predicateWild = [](const std::pair<const std::string, PropertyPersistenceInfo::Pointer>& x){return x.second.IsNotNull() && x.second->GetMimeTypeName() == PropertyPersistenceInfo::ANY_MIMETYPE_NAME(); };

    finding = std::find_if(infoRange.first, infoRange.second, predicateWild);
  }


  if (finding != infoRange.second)
  {
    result = finding->second;
  }

  return result;
}

bool mitk::PropertyPersistence::HasInfos(const std::string& propertyName)
{
  return !propertyName.empty()
    ? m_Infos.find(propertyName) != m_Infos.cend()
    : false;
}

void mitk::PropertyPersistence::RemoveAllInfos()
{
  m_Infos.clear();
}

void mitk::PropertyPersistence::RemoveInfos(const std::string& propertyName)
{
  if (!propertyName.empty())
    m_Infos.erase(propertyName);
}

void mitk::PropertyPersistence::RemoveInfos(const std::string& propertyName, const MimeTypeNameType& mime)
{
  auto itr = m_Infos.begin();
  while (itr != m_Infos.end())
  {
    if (itr->first == propertyName && itr->second.IsNotNull() && itr->second->GetMimeTypeName() == mime)
    {
      itr = m_Infos.erase(itr);
    }
    else {
      ++itr;
    }
  }
}


mitk::IPropertyPersistence* mitk::CreateTestInstancePropertyPersistence()
{
  return new PropertyPersistence();
};