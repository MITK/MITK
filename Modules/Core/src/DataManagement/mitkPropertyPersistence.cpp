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

#include <algorithm>
#include <regex>
#include <utility>

#include <mitkPropertyPersistence.h>

mitk::PropertyPersistence::PropertyPersistence()
{
}

mitk::PropertyPersistence::~PropertyPersistence()
{
}

bool mitk::PropertyPersistence::AddInfo(const PropertyPersistenceInfo *info, bool overwrite)
{
  if (!info)
  {
    return false;
  }

  if (info->GetName().empty())
  {
    return false;
  }

  mitk::PropertyPersistenceInfo::MimeTypeNameType mime = info->GetMimeTypeName();

  auto infoRange = m_InfoMap.equal_range(info->GetName());

  auto predicate = [mime](const std::pair<const std::string, mitk::PropertyPersistenceInfo::ConstPointer> &x) {
    return x.second.IsNotNull() && x.second->GetMimeTypeName() == mime;
  };

  auto finding = std::find_if(infoRange.first, infoRange.second, predicate);

  bool exists = finding != infoRange.second;
  bool result = false;

  if (!exists || overwrite)
  {
    if (exists && overwrite)
    {
      m_InfoMap.erase(finding);
    }
    result = true;
    m_InfoMap.insert(std::make_pair(info->GetName(), info));
  }

  return result;
}

mitk::PropertyPersistence::InfoMap mitk::PropertyPersistence::SelectInfo(const InfoMap &infoMap,
                                                                         const SelectFunctionType &selectFunction)
{
  InfoMap result;

  for (auto pos : infoMap)
  {
    if (selectFunction(pos))
    {
      result.insert(pos);
    }
  }

  return result;
};

mitk::PropertyPersistence::InfoResultType mitk::PropertyPersistence::GetInfo(const std::string &propertyName,
                                                                             bool allowNameRegEx) const
{
  SelectFunctionType select = [propertyName](const InfoMap::value_type &x) {
    return x.second.IsNotNull() && !x.second->IsRegEx() && x.second->GetName() == propertyName;
  };

  InfoMap selection = SelectInfo(m_InfoMap, select);

  InfoResultType result;
  for (const auto &pos : selection)
  {
    result.push_back(pos.second->UnRegExByName(propertyName).GetPointer());
  }

  if (allowNameRegEx)
  {
    select = [propertyName](const InfoMap::value_type &x) {
      if (x.second.IsNotNull() && x.second->IsRegEx())
      {
        std::regex ex(x.second->GetName());
        return std::regex_match(propertyName, ex);
      }
      return false;
    };

    selection = SelectInfo(m_InfoMap, select);

    for (const auto &pos : selection)
    {
      result.push_back(pos.second->UnRegExByName(propertyName).GetPointer());
    }
  }

  return result;
}

bool infoPredicate(const std::multimap<const std::string, mitk::PropertyPersistenceInfo::ConstPointer>::value_type &x,
                   const std::string &propertyName,
                   const std::string &mime)
{
  return x.second.IsNotNull() && !x.second->IsRegEx() && x.second->GetName() == propertyName &&
         x.second->GetMimeTypeName() == mime;
}

bool infoPredicateRegEx(
  const std::multimap<const std::string, mitk::PropertyPersistenceInfo::ConstPointer>::value_type &x,
  const std::string &propertyName,
  const std::string &mime)
{
  if (x.second.IsNotNull() && x.second->IsRegEx())
  {
    std::regex ex(x.second->GetName());
    return std::regex_match(propertyName, ex) && x.second->GetMimeTypeName() == mime;
  }
  return false;
}

mitk::PropertyPersistence::InfoResultType mitk::PropertyPersistence::GetInfo(const std::string &propertyName,
                                                                             const MimeTypeNameType &mime,
                                                                             bool allowMimeWildCard,
                                                                             bool allowNameRegEx) const
{
  SelectFunctionType select = [propertyName, mime](const InfoMap::value_type &x) {
    return infoPredicate(x, propertyName, mime);
  };

  InfoMap selection = SelectInfo(m_InfoMap, select);

  if (allowNameRegEx)
  {
    select = [propertyName, mime](const InfoMap::value_type &x) { return infoPredicateRegEx(x, propertyName, mime); };

    InfoMap regExSelection = SelectInfo(m_InfoMap, select);

    selection.insert(regExSelection.begin(), regExSelection.end());
  }

  if (selection.empty() && allowMimeWildCard)
  { // no perfect match => second run through with "any mime type"
    select = [propertyName](const InfoMap::value_type &x) {
      return infoPredicate(x, propertyName, PropertyPersistenceInfo::ANY_MIMETYPE_NAME());
    };

    selection = SelectInfo(m_InfoMap, select);

    if (allowNameRegEx)
    {
      select = [propertyName](const InfoMap::value_type &x) {
        return infoPredicateRegEx(x, propertyName, PropertyPersistenceInfo::ANY_MIMETYPE_NAME());
      };

      InfoMap regExSelection = SelectInfo(m_InfoMap, select);

      selection.insert(regExSelection.begin(), regExSelection.end());
    }
  }

  InfoResultType result;
  for (const auto &pos : selection)
  {
    result.push_back(pos.second->UnRegExByName(propertyName).GetPointer());
  }

  return result;
}

mitk::PropertyPersistence::InfoResultType mitk::PropertyPersistence::GetInfoByKey(const std::string &persistenceKey,
                                                                                  bool allowKeyRegEx) const
{
  InfoResultType result;

  for (const auto &pos : m_InfoMap)
  {
    if (pos.second.IsNotNull())
    {
      bool valid = pos.second->GetKey() == persistenceKey;
      if (!valid && pos.second->IsRegEx() && allowKeyRegEx)
      {
        std::regex ex(pos.second->GetKey());
        valid = std::regex_match(persistenceKey, ex);
      }

      if (valid)
      {
        result.push_back(pos.second->UnRegExByKey(persistenceKey).GetPointer());
      }
    }
  }

  return result;
}

bool mitk::PropertyPersistence::HasInfo(const std::string &propertyName, bool allowNameRegEx) const
{
  return !this->GetInfo(propertyName, allowNameRegEx).empty();
}

void mitk::PropertyPersistence::RemoveAllInfo()
{
  m_InfoMap.clear();
}

void mitk::PropertyPersistence::RemoveInfo(const std::string &propertyName)
{
  if (!propertyName.empty())
  {
    m_InfoMap.erase(propertyName);
  }
}

void mitk::PropertyPersistence::RemoveInfo(const std::string &propertyName, const MimeTypeNameType &mime)
{
  auto itr = m_InfoMap.begin();
  while (itr != m_InfoMap.end())
  {
    if (itr->first == propertyName && itr->second.IsNotNull() && itr->second->GetMimeTypeName() == mime)
    {
      itr = m_InfoMap.erase(itr);
    }
    else
    {
      ++itr;
    }
  }
}

mitk::IPropertyPersistence *mitk::CreateTestInstancePropertyPersistence()
{
  return new PropertyPersistence();
};