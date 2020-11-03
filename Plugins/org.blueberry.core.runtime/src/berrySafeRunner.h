/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef BERRYSAFERUNNER_H_
#define BERRYSAFERUNNER_H_

#include <org_blueberry_core_runtime_Export.h>

#include "berryISafeRunnable.h"

namespace berry {

/**
 * Runs the given ISafeRunnable in a protected mode: exceptions
 * thrown in the runnable are logged and passed to the runnable's
 * exception handler.  Such exceptions are not rethrown by this method.
 * <p>
 * This class can be used without OSGi running.
 * </p>
 */
class org_blueberry_core_runtime_EXPORT SafeRunner {

public:

  /**
   * Runs the given runnable in a protected mode.   Exceptions
   * thrown in the runnable are logged and passed to the runnable's
   * exception handler.  Such exceptions are not rethrown by this method.
   *
   * @param code the runnable to run
   */
  static void Run(ISafeRunnable::Pointer code);

private:

  static void HandleException(ISafeRunnable::Pointer code);
  static void HandleException(ISafeRunnable::Pointer code, const std::exception&);
  static void HandleException(ISafeRunnable::Pointer code, const ctkException& e);

};

}

#endif /* BERRYSAFERUNNER_H_ */
