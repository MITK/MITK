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

#include "mitkPropertyDescriptions.h"
#include <algorithm>
#include <regex>
#include <utility>

mitk::PropertyDescriptions::PropertyDescriptions()
{
}

mitk::PropertyDescriptions::~PropertyDescriptions()
{
}

bool mitk::PropertyDescriptions::AddDescription(const std::string &propertyName,
                                                const std::string &description,
                                                const std::string &className,
                                                bool overwrite)
{
  if (propertyName.empty())
    return false;

  DescriptionMap &descriptions = m_Descriptions[className];
  std::pair<DescriptionMapIterator, bool> ret = descriptions.insert(std::make_pair(propertyName, description));

  if (!ret.second && overwrite)
  {
    ret.first->second = description;
    ret.second = true;
  }

  return ret.second;
}

bool mitk::PropertyDescriptions::AddDescriptionRegEx(const std::string &propertyRegEx,
                                                     const std::string &description,
                                                     const std::string &className,
                                                     bool overwrite)
{
  if (propertyRegEx.empty())
    return false;

  try
  {
    std::regex checker(propertyRegEx); // no exception => valid we can change the info
  }
  catch (const std::regex_error &)
  {
    return false;
  }

  DescriptionMap &descriptions = m_DescriptionsRegEx[className];
  std::pair<DescriptionMapIterator, bool> ret = descriptions.insert(std::make_pair(propertyRegEx, description));

  if (!ret.second && overwrite)
  {
    ret.first->second = description;
    ret.second = true;
  }

  return ret.second;
}

std::string mitk::PropertyDescriptions::GetDescription(const std::string &propertyName,
                                                       const std::string &className,
                                                       bool allowNameRegEx) const
{
  if (!propertyName.empty())
  {
    auto descriptionsIter = m_Descriptions.find(className);

    if (descriptionsIter != m_Descriptions.cend())
    {
      auto iter = descriptionsIter->second.find(propertyName);

      if (iter != descriptionsIter->second.end())
        return iter->second;
    }
  }

  if (allowNameRegEx && !propertyName.empty())
  {
    auto selector = [propertyName](const DescriptionMap::value_type &x) {
      std::regex ex(x.first);
      return std::regex_match(propertyName, ex);
    };

    auto descriptionsIter = m_DescriptionsRegEx.find(className);

    if (descriptionsIter != m_DescriptionsRegEx.cend())
    {
      auto finding = std::find_if(descriptionsIter->second.cbegin(), descriptionsIter->second.cend(), selector);

      if (finding != descriptionsIter->second.cend())
        return finding->second;
    }
  }

  return "";
}

bool mitk::PropertyDescriptions::HasDescription(const std::string &propertyName,
                                                const std::string &className,
                                                bool allowNameRegEx) const
{
  if (!propertyName.empty())
  {
    auto descriptionsIter = m_Descriptions.find(className);

    if (descriptionsIter != m_Descriptions.cend())
    {
      auto iter = descriptionsIter->second.find(propertyName);

      if (iter != descriptionsIter->second.end())
        return true;
    }
  }

  if (allowNameRegEx && !propertyName.empty())
  {
    auto selector = [propertyName](const DescriptionMap::value_type &x) {
      std::regex ex(x.first);
      return std::regex_match(propertyName, ex);
    };

    auto descriptionsIter = m_DescriptionsRegEx.find(className);

    if (descriptionsIter != m_DescriptionsRegEx.cend())
    {
      auto finding = std::find_if(descriptionsIter->second.cbegin(), descriptionsIter->second.cend(), selector);

      if (finding != descriptionsIter->second.cend())
        return true;
    }
  }

  return false;
}

void mitk::PropertyDescriptions::RemoveAllDescriptions(const std::string &className)
{
  m_Descriptions[className].clear();
  m_DescriptionsRegEx[className].clear();
}

void mitk::PropertyDescriptions::RemoveDescription(const std::string &propertyName, const std::string &className)
{
  if (!propertyName.empty())
  {
    m_Descriptions[className].erase(propertyName);
    m_DescriptionsRegEx[className].erase(propertyName);
  }
}
