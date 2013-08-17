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
#include <utility>

mitk::PropertyDescriptions::PropertyDescriptions()
{
}

mitk::PropertyDescriptions::~PropertyDescriptions()
{
}

bool mitk::PropertyDescriptions::AddDescription(const std::string& propertyName, const std::string& description, bool overwrite)
{
  if (!propertyName.empty())
  {
    std::pair<std::map<std::string, std::string>::iterator, bool> ret = m_Descriptions.insert(std::make_pair(propertyName, description));

    if (!ret.second && overwrite)
    {
      ret.first->second = description;
      ret.second = true;
    }

    return ret.second;
  }

  return false;
}

std::string mitk::PropertyDescriptions::GetDescription(const std::string& propertyName) const
{
  if (!propertyName.empty())
  {
    std::map<std::string, std::string>::const_iterator iter = m_Descriptions.find(propertyName);

    if (iter != m_Descriptions.end())
      return iter->second;
  }

  return "";
}

bool mitk::PropertyDescriptions::HasDescription(const std::string& propertyName) const
{
  return !propertyName.empty()
    ? m_Descriptions.find(propertyName) != m_Descriptions.end()
    : false;
}

void mitk::PropertyDescriptions::RemoveAllDescriptions()
{
  m_Descriptions.clear();
}

void mitk::PropertyDescriptions::RemoveDescription(const std::string& propertyName)
{
  if (!propertyName.empty())
    m_Descriptions.erase(propertyName);
}
