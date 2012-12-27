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


#ifndef BERRYSAFERUNNER_H_
#define BERRYSAFERUNNER_H_

#include <org_blueberry_solstice_common_Export.h>

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
class BERRY_COMMON_RUNTIME SafeRunner {

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

  static void HandleException(ISafeRunnable::Pointer code, const std::exception& e = std::exception());

};

}

#endif /* BERRYSAFERUNNER_H_ */
