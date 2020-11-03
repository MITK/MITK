/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryAbstractContributionFactory.h"

namespace berry
{

AbstractContributionFactory::AbstractContributionFactory(
    const QString& location, const QString& namespaze)
  : location(location), namespaze(namespaze)
{
}

QString AbstractContributionFactory::GetLocation() const
{
  return location;
}

QString AbstractContributionFactory::GetNamespace() const
{
  return namespaze;
}

}
