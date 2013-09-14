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
#include <utility>

mitk::PropertyDescriptions::PropertyDescriptions()
{
}

mitk::PropertyDescriptions::~PropertyDescriptions()
{
}

bool mitk::PropertyDescriptions::AddDescription(const std::string& propertyName, const std::string& description, const std::string& className, bool overwrite)
{
  if (propertyName.empty())
    return false;

  DescriptionMap& descriptions = m_Descriptions[className];
  std::pair<DescriptionMapIterator, bool> ret = descriptions.insert(std::make_pair(propertyName, description));

  if (!ret.second && overwrite)
  {
    ret.first->second = description;
    ret.second = true;
  }

  return ret.second;
}

std::string mitk::PropertyDescriptions::GetDescription(const std::string& propertyName, const std::string& className)
{
  if (!propertyName.empty())
  {
    DescriptionMap& descriptions = m_Descriptions[className];
    DescriptionMapConstIterator iter = descriptions.find(propertyName);

    if (iter != descriptions.end())
      return iter->second;
  }

  return "";
}

bool mitk::PropertyDescriptions::HasDescription(const std::string& propertyName, const std::string& className)
{
  const DescriptionMap& descriptions = m_Descriptions[className];

  return !propertyName.empty()
    ? descriptions.find(propertyName) != descriptions.end()
    : false;
}

void mitk::PropertyDescriptions::RemoveAllDescriptions(const std::string& className)
{
  m_Descriptions[className].clear();
}

void mitk::PropertyDescriptions::RemoveDescription(const std::string& propertyName, const std::string& className)
{
  if (!propertyName.empty())
    m_Descriptions[className].erase(propertyName);
}
