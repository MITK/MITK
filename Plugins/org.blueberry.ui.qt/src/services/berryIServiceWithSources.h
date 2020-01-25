/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef BERRYISERVICEWITHSOURCES_H_
#define BERRYISERVICEWITHSOURCES_H_

#include "berryIDisposable.h"

namespace berry {

struct ISourceProvider;

/**
 * <p>
 * A service that responds to changes in one or more sources. These sources can
 * be plugged into the service. Sources represent a common event framework for
 * services.
 * </p>
 * <p>
 * Clients must not extend or implement.
 * </p>
 *
 */
struct IServiceWithSources : public IDisposable {

  berryObjectMacro(berry::IServiceWithSources);

  ~IServiceWithSources() override;

  /**
   * Adds a source provider to this service. A source provider will notify the
   * service when the source it provides changes. An example of a source might
   * be an active editor or the current selection. This amounts to a pluggable
   * state tracker for the service.
   *
   * @param provider
   *            The provider to add; must not be <code>null</code>.
   */
  virtual void AddSourceProvider(const SmartPointer<ISourceProvider>& provider) = 0;

  /**
   * Removes a source provider from this service. Most of the time, this
   * method call is not required as source providers typically share the same
   * life span as the workbench itself.
   *
   * @param provider
   *            The provider to remove; must not be <code>null</code>.
   */
  virtual void RemoveSourceProvider(const SmartPointer<ISourceProvider>& provider) = 0;
};

}

#endif /* BERRYISERVICEWITHSOURCES_H_ */
