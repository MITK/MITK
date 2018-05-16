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

#include <mitkIdentifiable.h>
#include <mitkUIDGenerator.h>

struct mitk::Identifiable::Impl
{
  UIDType uid;
};

mitk::Identifiable::Identifiable()
  : m_Impl(new Impl)
{
  UIDGenerator generator;
  m_Impl->uid = generator.GetUID();
}

mitk::Identifiable::Identifiable(const UIDType &uid)
  : m_Impl(new Impl)
{
  m_Impl->uid = uid;
}

mitk::Identifiable::Identifiable(Identifiable &&other) noexcept
  : m_Impl(other.m_Impl)
{
  other.m_Impl = nullptr;
}

mitk::Identifiable::~Identifiable()
{
  delete m_Impl;
}

mitk::Identifiable & mitk::Identifiable::operator =(Identifiable &&other) noexcept
{
  delete m_Impl;
  m_Impl = other.m_Impl;
  other.m_Impl = nullptr;

  return *this;
}

mitk::Identifiable::UIDType mitk::Identifiable::GetUID() const
{
  return m_Impl->uid;
}

 void mitk::Identifiable::SetUID(const UIDType &uid)
{
  m_Impl->uid = uid;
}
