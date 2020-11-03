/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYIADAPTERMANAGERPROVIDER_H_
#define BERRYIADAPTERMANAGERPROVIDER_H_

namespace berry {

class AdapterManager;

/**
 * The callback interface for the elements desiring to lazily supply
 * information to the adapter manager.
 *
 * @since org.blueberry.core.runtime 3.2
 */
struct IAdapterManagerProvider {

  /**
   * Add factories. The method called before the AdapterManager starts
   * using factories.
   *
   * @param adapterManager the adapter manager that is about to be used
   * @return <code>true</code> if factories were added; <code>false</code>
   * if no factories were added in this method call.
   */
  bool AddFactories(AdapterManager* adapterManager);
};

}  // namespace berry

#endif /*BERRYIADAPTERMANAGERPROVIDER_H_*/
