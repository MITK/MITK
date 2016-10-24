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

#include "mitkPropertyFilter.h"
#include <algorithm>

namespace mitk
{
  class PropertyFilter::Impl
  {
  public:
    Impl();
    ~Impl();

    void AddEntry(const std::string &propertyName, PropertyFilter::List list);
    std::map<std::string, BaseProperty::Pointer> Apply(
      const std::map<std::string, BaseProperty::Pointer> &propertyMap) const;
    bool HasEntry(const std::string &propertyName, PropertyFilter::List list) const;
    bool IsEmpty() const;
    void RemoveAllEntries(PropertyFilter::List list);
    void RemoveEntry(const std::string &propertyName, PropertyFilter::List list);

  private:
    void AddEntry(const std::string &propertyName, std::vector<std::string> &list);
    bool HasEntry(const std::string &propertyName, const std::vector<std::string> &list) const;
    void RemoveAllEntries(std::vector<std::string> &list);
    void RemoveEntry(const std::string &propertyName, std::vector<std::string> &list);

    std::vector<std::vector<std::string>> m_Lists;
  };

  PropertyFilter::Impl::Impl() : m_Lists(2) {}
  PropertyFilter::Impl::~Impl() {}
  void mitk::PropertyFilter::Impl::AddEntry(const std::string &propertyName, List list)
  {
    this->AddEntry(propertyName, m_Lists.at(list));
  }

  void PropertyFilter::Impl::AddEntry(const std::string &propertyName, std::vector<std::string> &list)
  {
    if (std::find(list.begin(), list.end(), propertyName) == list.end())
      list.push_back(propertyName);
  }

  std::map<std::string, BaseProperty::Pointer> PropertyFilter::Impl::Apply(
    const std::map<std::string, BaseProperty::Pointer> &propertyMap) const
  {
    std::map<std::string, BaseProperty::Pointer> ret =
      !m_Lists[Whitelist].empty() ? std::map<std::string, BaseProperty::Pointer>() : propertyMap;

    if (!m_Lists[Whitelist].empty())
    {
      auto end = propertyMap.end();

      for (auto iter = propertyMap.begin(); iter != end; ++iter)
      {
        if (std::find(m_Lists[Whitelist].begin(), m_Lists[Whitelist].end(), iter->first) != m_Lists[Whitelist].end())
          ret.insert(*iter);
      }
    }

    if (!m_Lists[Blacklist].empty())
    {
      auto end = m_Lists[Blacklist].end();

      for (auto iter = m_Lists[Blacklist].begin(); iter != end; ++iter)
        ret.erase(*iter);
    }

    return ret;
  }

  bool mitk::PropertyFilter::Impl::HasEntry(const std::string &propertyName, List list) const
  {
    return this->HasEntry(propertyName, m_Lists.at(list));
  }

  bool PropertyFilter::Impl::HasEntry(const std::string &propertyName, const std::vector<std::string> &list) const
  {
    return std::find(list.begin(), list.end(), propertyName) != list.end();
  }

  bool PropertyFilter::Impl::IsEmpty() const
  {
    auto end = m_Lists.end();

    for (auto iter = m_Lists.begin(); iter != end; ++iter)
    {
      if (!iter->empty())
        return false;
    }

    return true;
  }

  void mitk::PropertyFilter::Impl::RemoveAllEntries(List list) { this->RemoveAllEntries(m_Lists.at(list)); }
  void PropertyFilter::Impl::RemoveAllEntries(std::vector<std::string> &list) { list.clear(); }
  void mitk::PropertyFilter::Impl::RemoveEntry(const std::string &propertyName, List list)
  {
    this->RemoveEntry(propertyName, m_Lists.at(list));
  }

  void PropertyFilter::Impl::RemoveEntry(const std::string &propertyName, std::vector<std::string> &list)
  {
    auto iter = std::find(list.begin(), list.end(), propertyName);

    if (iter != list.end())
      list.erase(iter);
  }
}

mitk::PropertyFilter::PropertyFilter() : m_Impl(new Impl)
{
}

mitk::PropertyFilter::~PropertyFilter()
{
  delete m_Impl;
}

mitk::PropertyFilter::PropertyFilter(const mitk::PropertyFilter &other) : m_Impl(new Impl(*other.m_Impl))
{
}

mitk::PropertyFilter &mitk::PropertyFilter::operator=(mitk::PropertyFilter other)
{
  std::swap(this->m_Impl, other.m_Impl);
  return *this;
}

void mitk::PropertyFilter::AddEntry(const std::string &propertyName, List list)
{
  if (!propertyName.empty())
    m_Impl->AddEntry(propertyName, list);
}

std::map<std::string, mitk::BaseProperty::Pointer> mitk::PropertyFilter::Apply(
  const std::map<std::string, mitk::BaseProperty::Pointer> &propertyMap) const
{
  return m_Impl->Apply(propertyMap);
}

bool mitk::PropertyFilter::HasEntry(const std::string &propertyName, List list) const
{
  return !propertyName.empty() ? m_Impl->HasEntry(propertyName, list) : false;
}

bool mitk::PropertyFilter::IsEmpty() const
{
  return m_Impl->IsEmpty();
}

void mitk::PropertyFilter::RemoveAllEntries(List list)
{
  m_Impl->RemoveAllEntries(list);
}

void mitk::PropertyFilter::RemoveEntry(const std::string &propertyName, List list)
{
  if (!propertyName.empty())
    m_Impl->RemoveEntry(propertyName, list);
}
