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


#include "berryAbstractMenuAdditionCacheEntry.h"

#include <berryIConfigurationElement.h>

namespace berry {

AbstractMenuAdditionCacheEntry::AbstractMenuAdditionCacheEntry(
    const QString& location, const QString& namespaze,
    const SmartPointer<IConfigurationElement>& element)
  : AbstractContributionFactory(location, namespaze), additionElement(element)
{
}

SmartPointer<IConfigurationElement> AbstractMenuAdditionCacheEntry::GetConfigElement() const
{
  return additionElement;
}

}
