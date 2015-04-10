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

#include "berrySimpleExtensionPointFilter.h"

#include <berryIExtensionPoint.h>

namespace berry {

SimpleExtensionPointFilter::SimpleExtensionPointFilter(const QString& id)
  : m_Id(id)
{}

bool SimpleExtensionPointFilter::Matches(const IExtensionPoint* target) const
{
  return m_Id.isEmpty() ? true : m_Id == target->GetUniqueIdentifier();
}

}
