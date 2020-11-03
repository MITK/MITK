/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkPropertyFilters.h"
#include <mitkPropertyFilter.h>
#include <utility>

mitk::PropertyFilters::PropertyFilters()
{
}

mitk::PropertyFilters::~PropertyFilters()
{
}

bool mitk::PropertyFilters::AddFilter(const PropertyFilter &filter, const std::string &className, bool overwrite)
{
  if (!filter.IsEmpty())
  {
    std::pair<std::map<std::string, PropertyFilter>::iterator, bool> ret =
      m_Filters.insert(std::make_pair(className, filter));

    if (!ret.second && overwrite)
    {
      ret.first->second = filter;
      ret.second = true;
    }

    return ret.second;
  }

  return false;
}

std::map<std::string, mitk::BaseProperty::Pointer> mitk::PropertyFilters::ApplyFilter(
  const std::map<std::string, BaseProperty::Pointer> &propertyMap, const std::string &className) const
{
  std::map<std::string, BaseProperty::Pointer> ret = propertyMap;
  PropertyFilter filter = this->GetFilter("");

  if (!filter.IsEmpty())
    ret = filter.Apply(ret);

  if (!className.empty())
  {
    filter = this->GetFilter(className);

    if (!filter.IsEmpty())
      ret = filter.Apply(ret);
  }

  return ret;
}

mitk::PropertyFilter mitk::PropertyFilters::GetFilter(const std::string &className) const
{
  auto iter = m_Filters.find(className);

  if (iter != m_Filters.end())
    return iter->second;

  return PropertyFilter();
}

bool mitk::PropertyFilters::HasFilter(const std::string &className) const
{
  return m_Filters.find(className) != m_Filters.end();
}

void mitk::PropertyFilters::RemoveAllFilters()
{
  m_Filters.clear();
}

void mitk::PropertyFilters::RemoveFilter(const std::string &className)
{
  m_Filters.erase(className);
}
