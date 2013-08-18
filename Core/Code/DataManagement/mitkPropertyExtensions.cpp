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

bool mitk::PropertyExtensions::AddExtension(const std::string& propertyName, PropertyExtension* extension, const std::string& className, bool overwrite)
{
  if (propertyName.empty())
    return false;

  ExtensionMap& extensions = m_Extensions[className];
  std::pair<ExtensionMapIterator, bool> ret = extensions.insert(std::make_pair(propertyName, extension));

  if (!ret.second && overwrite)
  {
    ret.first->second = extension;
    ret.second = true;
  }

  return ret.second;
}

mitk::PropertyExtension* mitk::PropertyExtensions::GetExtension(const std::string& propertyName, const std::string& className)
{
  if (!propertyName.empty())
  {
    ExtensionMap& extensions = m_Extensions[className];
    ExtensionMapConstIterator iter = extensions.find(propertyName);

    if (iter != extensions.end())
      return iter->second;
  }

  return NULL;
}

bool mitk::PropertyExtensions::HasExtension(const std::string& propertyName, const std::string& className)
{
  const ExtensionMap& extensions = m_Extensions[className];

  return !propertyName.empty()
    ? extensions.find(propertyName) != extensions.end()
    : false;
}

void mitk::PropertyExtensions::RemoveAllExtensions(const std::string& className)
{
  ExtensionMap& extensions = m_Extensions[className];

  std::for_each(extensions.begin(), extensions.end(), DeleteExtension());
  extensions.clear();
}

void mitk::PropertyExtensions::RemoveExtension(const std::string& propertyName, const std::string& className)
{
  if (!propertyName.empty())
  {
    ExtensionMap& extensions = m_Extensions[className];

    delete extensions[propertyName];
    extensions.erase(propertyName);
  }
}
