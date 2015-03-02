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
