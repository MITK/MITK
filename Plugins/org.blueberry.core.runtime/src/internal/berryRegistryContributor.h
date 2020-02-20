/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYREGISTRYCONTRIBUTOR_H
#define BERRYREGISTRYCONTRIBUTOR_H

#include "berryIContributor.h"

namespace berry {

/**
 * This class describes a registry contributor which is an entity that supplies information
 * to the extension registry. Depending on the registry strategy, contributor might delegate
 * some of its functionality to a "host" contributor. For instance, OSGi registry strategy
 * uses "host" contributor to delegate some functionality from fragments to plug-ins.
 * <p>
 * This class can be instantiated by the registry Service Providers.
 * </p><p>
 * This class is not intended to be extended.
 * </p><p>
 * <b>Note:</b> This class/interface is part of an interim API that is still under
 * development and expected to change significantly before reaching stability.
 * It is being made available at this early stage to solicit feedback from pioneering
 * adopters on the understanding that any code that uses this API will almost certainly
 * be broken (repeatedly) as the API evolves.
 * </p>
 * @noextend This class is not intended to be subclassed by clients.
 */
class RegistryContributor : public IContributor
{

private:

  /**
   * Actual ID of the contributor (e.g., "12"). IDs are expected to be unique in the workspace.
   */
  QString actualContributorId;

  /**
   * Actual name of the contributor (e.g., "org.eclipse.core.runtime.fragment").
   */
  QString actualContributorName;

  /**
   * ID associated with the entity "in charge" of the contributor (e.g., "1"). IDs are expected
   * to be unique in the workspace. If contributor does not rely on a host, this value should be
   * the same as the actual contributor ID.
   */
  QString hostId;

  /**
   * Name of the entity "in charge" of the contributor (e.g. "org.eclipse.core.runtime").
   * If contributor does not rely on a host, this value should be the same as the actual
   * contributor name.
   */
  QString hostName;

public:

  berryObjectMacro(berry::RegistryContributor);

  /**
   * Constructor for the registry contributor.
   * <p>
   * The actual ID is a string identifier for the contributor (e.g., "12") and is expected
   * to be unique within the workspace. The actual ID of the contributor must not
   * be <code>null</code>.
   * </p><p>
   * The actual name is the name associated with the contributor
   * (e.g., "org.eclipse.core.runtime.fragment"). The actual name of the contributor must
   * not be <code>null</code>.
   * </p><p>
   * The host ID is the identifier associated with the entity "in charge" of the contributor
   * (e.g., "1"). IDs are expected to be unique in the workspace. If contributor does not
   * rely on a host, then <code>null</code> should be used as the host ID.
   * </p><p>
   * The host name is the name of the entity "in charge" of the contributor
   * (e.g., "org.eclipse.core.runtime"). If contributor does not rely on a host, then
   * <code>null</code> should be used as the host name.
   * </p><p>
   * There should be 1-to-1 mapping between the contributor and the contibutor ID.
   * The IDs (either actual or host) can not be re-used in the same registry.
   * For example, if ID of 12 was used to identify contributorA, the ID of 12 can not
   * be used to identify contributorB or a host for the contributorC.
   * </p>
   * @param actualId contributor identifier
   * @param actualName name of the contributor
   * @param hostId id associated with the host, or <code>null</code>
   * @param hostName name of the host, or <code>null</code>
   */
  RegistryContributor(const QString& actualId, const QString& actualName,
                      const QString& hostId, const QString& hostName);

  /**
   * Provides actual ID associated with the registry contributor (e.g., "12"). IDs are expected
   * to be unique in the workspace.
   *
   * @return actual ID of the registry contributor
   */
  QString GetActualId() const;

  /**
   * Provides actual name of the registry contributor (e.g., "org.eclipe.core.runtime.fragment").
   *
   * @return actual name of the registry contributor
   */
  QString GetActualName() const;

  /**
   * Provides ID associated with the entity "in charge" of the contributor (e.g., "1"). IDs are expected
   * to be unique in the workspace. If contributor does not rely on a host, this value should be
   * the same as the actual contributor ID.
   *
   * @return id of the registry contributor
   */
  QString GetId() const;

  /**
   * Provides name of the entity "in charge" of the contributor (e.g., "org.eclipse.core.runtime").
   * If contributor does not rely on a host, this value should be the same as the actual contributor name.
   *
   * @return name of the registry contributor
   */
  QString GetName() const override;

};

}

#endif // BERRYREGISTRYCONTRIBUTOR_H
