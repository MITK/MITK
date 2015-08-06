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
    return false;

  auto ret = m_Infos.insert(std::make_pair(propertyName, info));

  if (!ret.second && overwrite)
  {
    ret.first->second = info;
    ret.second = true;
  }

  return ret.second;
}

mitk::PropertyPersistenceInfo::Pointer mitk::PropertyPersistence::GetInfo(const std::string& propertyName)
{
  if (!propertyName.empty())
  {
    InfoMapConstIterator iter = m_Infos.find(propertyName);

    if (iter != m_Infos.cend())
      return iter->second;
  }

  return nullptr;
}

bool mitk::PropertyPersistence::HasInfo(const std::string& propertyName)
{
  return !propertyName.empty()
    ? m_Infos.find(propertyName) != m_Infos.cend()
    : false;
}

void mitk::PropertyPersistence::RemoveAllInfos()
{
  m_Infos.clear();
}

void mitk::PropertyPersistence::RemoveInfo(const std::string& propertyName)
{
  if (!propertyName.empty())
    m_Infos.erase(propertyName);
}
