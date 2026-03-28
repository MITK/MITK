/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef USTRACKEDSERVICELISTENER_H
#define USTRACKEDSERVICELISTENER_H

#include <usServiceEvent.h>

namespace us {

/**
 * This class is not intended to be used directly. It is exported to support
 * the CppMicroServices module system.
 */
struct TrackedServiceListener
{
  virtual ~TrackedServiceListener() {}

  /**
   * Slot connected to service events for the
   * <code>ServiceTracker</code> class. This method must NOT be
   * synchronized to avoid deadlock potential.
   *
   * @param event <code>ServiceEvent</code> object from the framework.
   */
  virtual void ServiceChanged(const ServiceEvent event) = 0;

};

}

#endif // USTRACKEDSERVICELISTENER_H
