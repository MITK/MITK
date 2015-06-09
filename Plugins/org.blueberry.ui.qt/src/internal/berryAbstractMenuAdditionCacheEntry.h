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


#ifndef BERRYABSTRACTMENUADDITIONCACHEENTRY_H
#define BERRYABSTRACTMENUADDITIONCACHEENTRY_H

#include "berryAbstractContributionFactory.h"

namespace berry {

struct IConfigurationElement;

/**
 * Base class for factories derived from extensions.
 */
class AbstractMenuAdditionCacheEntry : public AbstractContributionFactory
{

private:

  SmartPointer<IConfigurationElement> additionElement;

public:

  berryObjectMacro(berry::AbstractMenuAdditionCacheEntry)

  /**
   * Create a new instance of this class.
   *
   * @param location the location URI
   * @param namespace the namespace
   * @param element the declaring configuration element
   */
  AbstractMenuAdditionCacheEntry(const QString& location, const QString& namespaze,
                                 const SmartPointer<IConfigurationElement>& element);

  /**
   * Return the configuration element.
   *
   * @return the configuration element
   */
  SmartPointer<IConfigurationElement> GetConfigElement() const;
};

}

#endif // BERRYABSTRACTMENUADDITIONCACHEENTRY_H
