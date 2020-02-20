/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef BERRYISAFERUNNABLERUNNER_H_
#define BERRYISAFERUNNABLERUNNER_H_

#include <org_blueberry_ui_qt_Export.h>

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
struct BERRY_UI_QT ISafeRunnableRunner : public Object
{

public:

  berryObjectMacro(berry::ISafeRunnableRunner);

  ~ISafeRunnableRunner() override;

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
