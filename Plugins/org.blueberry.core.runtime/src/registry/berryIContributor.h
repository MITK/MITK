/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYICONTRIBUTOR_H
#define BERRYICONTRIBUTOR_H

#include "berryObject.h"

#include <org_blueberry_core_runtime_Export.h>

namespace berry {

/**
 * This interface describes a registry contributor - an entity that supplies information
 * to the extension registry.
 * <p>
 * Registry contributor objects can be obtained by calling {@link IExtensionPoint#GetContributor()},
 * {@link IExtension#GetContributor()}, and {@link IConfigurationElement#GetContributor()}.
 * Alternatively, a contributor factory appropriate for the registry in use can be called to directly
 * obtain an IContributor object.
 * </p><p>
 * This interface is not intended to be implemented or extended by clients.
 * </p>
 * @see ContributorFactoryOSGi
 * @see ContributorFactorySimple
 *
 * @noimplement This interface is not intended to be implemented by clients.
 * @noextend This interface is not intended to be extended by clients.
 */
struct org_blueberry_core_runtime_EXPORT IContributor : public Object
{
  berryObjectMacro(berry::IContributor);

  ~IContributor() override;

  /**
   * Provides name of the contributor (e.g., "org.eclipse.core.runtime").
   *
   * @return name of the registry contributor
   */
  virtual QString GetName() const = 0;
};

}

#endif // BERRYICONTRIBUTOR_H
