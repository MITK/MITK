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

#include "mitkPropertyExtensions.h"
#include <mitkPropertyExtension.h>
#include <algorithm>
#include <utility>

class DeleteExtension
{
public:
  void operator()(const std::pair<std::string, mitk::PropertyExtension*>& element)
  {
    delete element.second;
  }
};

mitk::PropertyExtensions::PropertyExtensions()
{
}

mitk::PropertyExtensions::~PropertyExtensions()
{
}

bool mitk::PropertyExtensions::AddExtension(const std::string& propertyName, PropertyExtension* extension, bool overwrite)
{
  if (!propertyName.empty())
  {
    std::pair<std::map<std::string, PropertyExtension*>::iterator, bool> ret = m_Extensions.insert(std::make_pair(propertyName, extension));

    if (!ret.second && overwrite)
    {
      ret.first->second = extension;
      ret.second = true;
    }

    return ret.second;
  }

  return false;
}

mitk::PropertyExtension* mitk::PropertyExtensions::GetExtension(const std::string& propertyName) const
{
  if (!propertyName.empty())
  {
    std::map<std::string, PropertyExtension*>::const_iterator iter = m_Extensions.find(propertyName);

    if (iter != m_Extensions.end())
      return iter->second;
  }

  return NULL;
}

bool mitk::PropertyExtensions::HasExtension(const std::string& propertyName) const
{
  return !propertyName.empty()
    ? m_Extensions.find(propertyName) != m_Extensions.end()
    : false;
}

void mitk::PropertyExtensions::RemoveAllExtensions()
{
  std::for_each(m_Extensions.begin(), m_Extensions.end(), DeleteExtension());
  m_Extensions.clear();
}

void mitk::PropertyExtensions::RemoveExtension(const std::string& propertyName)
{
  if (!propertyName.empty())
  {
    delete m_Extensions[propertyName];
    m_Extensions.erase(propertyName);
  }
}
