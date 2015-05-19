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

#ifndef BERRYCONTRIBUTORFACTORY_H
#define BERRYCONTRIBUTORFACTORY_H

#include <org_blueberry_core_runtime_Export.h>

#include <QSharedPointer>

class ctkPlugin;

namespace berry {

template<typename A> class SmartPointer;

struct IContributor;

/**
 * The contributor factory creates new registry contributors for use in the registry.
 * <p>
 * This class can not be extended or instantiated by clients.
 * </p>
 * @noinstantiate This class is not intended to be instantiated by clients.
 * @noextend This class is not intended to be subclassed by clients.
 */
class org_blueberry_core_runtime_EXPORT ContributorFactory
{

public:

  /**
   * Creates registry contributor object based on a Plugin. The plugin must not
   * be <code>null</code>.
   *
   * @param contributor plugin associated with the contribution
   * @return new registry contributor based on the Plugin
   */
  static SmartPointer<IContributor> CreateContributor(const QSharedPointer<ctkPlugin>& contributor);

  /**
   * Returns the CTK plugin used to define this contributor.
   *
   * <p>The method may return null if the contributor is not based on a plugin,
   * if the plugin can't be found, or if the plugin is presently unresolved or
   * uninstalled.</p>
   *
   * @param contributor plugin-based registry contributor
   * @return the actual CTK plugin associated with this contributor
   */
  static QSharedPointer<ctkPlugin> Resolve(const SmartPointer<IContributor>& contributor);

private:

  // not implemented
  ContributorFactory();
  ~ContributorFactory();
};

}

#endif // BERRYCONTRIBUTORFACTORY_H
