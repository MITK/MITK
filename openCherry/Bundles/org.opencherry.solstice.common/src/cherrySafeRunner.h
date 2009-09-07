/*=========================================================================

 Program:   openCherry Platform
 Language:  C++
 Date:      $Date$
 Version:   $Revision$

 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/


#ifndef CHERRYSAFERUNNER_H_
#define CHERRYSAFERUNNER_H_

#include "cherryCommonRuntimeDll.h"

#include "cherryISafeRunnable.h"

namespace cherry {

/**
 * Runs the given ISafeRunnable in a protected mode: exceptions
 * thrown in the runnable are logged and passed to the runnable's
 * exception handler.  Such exceptions are not rethrown by this method.
 * <p>
 * This class can be used without OSGi running.
 * </p>
 */
class CHERRY_COMMON_RUNTIME SafeRunner {

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

#endif /* CHERRYSAFERUNNER_H_ */
