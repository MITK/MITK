/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
