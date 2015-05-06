/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

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
