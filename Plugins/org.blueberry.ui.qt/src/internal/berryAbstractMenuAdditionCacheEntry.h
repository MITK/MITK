/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


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

  berryObjectMacro(berry::AbstractMenuAdditionCacheEntry);

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
