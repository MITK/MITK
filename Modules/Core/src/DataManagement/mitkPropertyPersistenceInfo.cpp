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

#include <mitkPropertyPersistenceInfo.h>

namespace mitk
{
  struct PropertyPersistenceInfo::Impl
  {
    explicit Impl(const std::string& key);
    ~Impl();

    std::string Key;
  };

  PropertyPersistenceInfo::Impl::Impl(const std::string& key)
    : Key(key)
  {
  }

  PropertyPersistenceInfo::Impl::~Impl()
  {
  }
}

mitk::PropertyPersistenceInfo::PropertyPersistenceInfo(const std::string& key)
  : m_Impl(new Impl(key))
{
}

mitk::PropertyPersistenceInfo::~PropertyPersistenceInfo()
{
  delete m_Impl;
}

std::string mitk::PropertyPersistenceInfo::GetKey() const
{
  return m_Impl->Key;
}

void mitk::PropertyPersistenceInfo::SetKey(const std::string& key)
{
  m_Impl->Key = key;
}
