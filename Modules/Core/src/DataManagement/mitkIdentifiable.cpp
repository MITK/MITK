/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkIdentifiable.h>
#include <mitkUIDGenerator.h>

#include <sstream>

struct mitk::Identifiable::Impl
{
  UIDType uid;
  UIDType m_RuntimeUID;
};

mitk::Identifiable::Identifiable()
  : m_Impl(new Impl)
{
  UIDGenerator generator;
  m_Impl->uid = generator.GetUID();

  std::ostringstream ss;
  ss << static_cast<const void*>(this);
  m_Impl->m_RuntimeUID = ss.str();
}

mitk::Identifiable::Identifiable(const UIDType &uid)
  : m_Impl(new Impl)
{
  m_Impl->uid = uid;

  std::ostringstream ss;
  ss << static_cast<const void*>(this);
  m_Impl->m_RuntimeUID = ss.str();
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
  m_Impl->m_RuntimeUID = std::move(other.m_Impl->m_RuntimeUID);

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

mitk::Identifiable::UIDType mitk::Identifiable::GetRuntimeUID() const noexcept
{
  return m_Impl->m_RuntimeUID;
}
