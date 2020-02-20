/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryIExtensionPointFilter.h"

namespace berry {

IExtensionPointFilter::IExtensionPointFilter(const IExtensionPointFilter::Concept* concept)
  : m_Self(concept)
{
}

IExtensionPointFilter::Concept::~Concept()
{

}

bool IExtensionPointFilter::IsNull() const
{
  return m_Self.get() == nullptr;
}

const IExtensionPointFilter::Concept* IExtensionPointFilter::GetConcept() const
{
  return m_Self.get();
}

bool IExtensionPointFilter::Matches(const IExtensionPoint* target) const
{
  return m_Self->Matches(target);
}

}
