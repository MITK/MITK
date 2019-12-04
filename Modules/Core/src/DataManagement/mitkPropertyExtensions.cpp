/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkPropertyExtensions.h"
#include <algorithm>
#include <utility>

mitk::PropertyExtensions::PropertyExtensions()
{
}

mitk::PropertyExtensions::~PropertyExtensions()
{
}

bool mitk::PropertyExtensions::AddExtension(const std::string &propertyName,
                                            PropertyExtension::Pointer extension,
                                            const std::string &className,
                                            bool overwrite)
{
  if (propertyName.empty())
    return false;

  ExtensionMap &extensions = m_Extensions[className];
  std::pair<ExtensionMapIterator, bool> ret = extensions.insert(std::make_pair(propertyName, extension));

  if (!ret.second && overwrite)
  {
    ret.first->second = extension;
    ret.second = true;
  }

  return ret.second;
}

mitk::PropertyExtension::Pointer mitk::PropertyExtensions::GetExtension(const std::string &propertyName,
                                                                        const std::string &className)
{
  if (!propertyName.empty())
  {
    ExtensionMap &extensions = m_Extensions[className];
    ExtensionMapConstIterator iter = extensions.find(propertyName);

    if (iter != extensions.end())
      return iter->second;
  }

  return nullptr;
}

bool mitk::PropertyExtensions::HasExtension(const std::string &propertyName, const std::string &className)
{
  const ExtensionMap &extensions = m_Extensions[className];

  return !propertyName.empty() ? extensions.find(propertyName) != extensions.end() : false;
}

void mitk::PropertyExtensions::RemoveAllExtensions(const std::string &className)
{
  m_Extensions[className].clear();
}

void mitk::PropertyExtensions::RemoveExtension(const std::string &propertyName, const std::string &className)
{
  if (!propertyName.empty())
    m_Extensions[className].erase(propertyName);
}
