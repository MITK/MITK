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


#ifndef BERRYISAFERUNNABLERUNNER_H_
#define BERRYISAFERUNNABLERUNNER_H_

#include <org_blueberry_ui_Export.h>

#include <berryObject.h>
#include <berryMacros.h>

#include <berryISafeRunnable.h>

namespace berry {

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
struct BERRY_UI ISafeRunnableRunner : public Object {

public:

  berryInterfaceMacro(ISafeRunnableRunner, berry)

  ~ISafeRunnableRunner();

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

#endif /* BERRYISAFERUNNABLERUNNER_H_ */
