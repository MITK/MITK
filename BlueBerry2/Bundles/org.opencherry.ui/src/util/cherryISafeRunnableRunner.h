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


#ifndef CHERRYISAFERUNNABLERUNNER_H_
#define CHERRYISAFERUNNABLERUNNER_H_

#include "../cherryUiDll.h"

#include <cherryObject.h>
#include <cherryMacros.h>

#include <cherryISafeRunnable.h>

namespace cherry {

/**
 * Runs a safe runnables.
 * <p>
 * Clients may provide their own implementation to change
 * how safe runnables are run from within the workbench.
 * </p>
 *
 * @see SafeRunnable#GetRunner()
 * @see SafeRunnable#SetRunner(ISafeRunnableRunner::Pointer)
 * @see SafeRunnable#Run(ISafeRunnable::Pointer)
 */
struct CHERRY_UI ISafeRunnableRunner : public Object {

public:

  cherryInterfaceMacro(ISafeRunnableRunner, cherry)

  /**
   * Runs the runnable.  All <code>ISafeRunnableRunners</code> must catch any exception
   * thrown by the <code>ISafeRunnable</code> and pass the exception to
   * <code>ISafeRunnable::HandleException()</code>.
   * @param code the code executed as a save runnable
   *
   * @see SafeRunnable#Run(ISafeRunnable::Pointer)
   */
  virtual void Run(ISafeRunnable::Pointer code) = 0;

};

}

#endif /* CHERRYISAFERUNNABLERUNNER_H_ */
