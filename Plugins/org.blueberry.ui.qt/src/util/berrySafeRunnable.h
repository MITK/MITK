/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef BERRYSAFERUNNABLE_H_
#define BERRYSAFERUNNABLE_H_

#include <berryISafeRunnable.h>
#include "berryISafeRunnableRunner.h"

namespace berry {

/**
 * Implements a default implementation of ISafeRunnable. The default
 * implementation of <code>HandleException</code> opens a dialog to show any
 * errors as they accumulate.
 * <p>
 * This may be executed on any thread.
 */
class BERRY_UI_QT SafeRunnable : public ISafeRunnable {

private:

  static bool ignoreErrors;

  static ISafeRunnableRunner::Pointer runner;

  QString message;

   /**
   * Creates the default safe runnable runner.
   *
   * @return the default safe runnable runner
   */
  static ISafeRunnableRunner::Pointer CreateDefaultRunner();

public:

  using ISafeRunnable::Run;

  /**
   * Creates a new instance of SafeRunnable with the given error message.
   *
   * @param message
   *            the error message to use
   */
  SafeRunnable(const QString& message = "");

  /*
   * @see ISafeRunnable#HandleException(const ctkException&)
   */
  void HandleException(const ctkException& e) override;

  /**
   * Flag to avoid interactive error dialogs during automated testing.
   *
   * @return true if errors should be ignored
   *
   */
  static bool GetIgnoreErrors();

  /**
   * Flag to avoid interactive error dialogs during automated testing.
   *
   * @param flag
   *            set to true if errors should be ignored
   */
  static void SetIgnoreErrors(bool flag);

  /**
   * Returns the safe runnable runner.
   *
   * @return the safe runnable runner
   *
   */
  static ISafeRunnableRunner::Pointer GetRunner();

  /**
   * Sets the safe runnable runner.
   *
   * @param runner
   *            the runner to set, or <code>null</code> to reset to the
   *            default runner
   */
  static void SetRunner(ISafeRunnableRunner::Pointer runner);

  /**
   * Runs the given safe runnable using the safe runnable runner. This is a
   * convenience method, equivalent to:
   * <code>SafeRunnable#GetRunner()->Run(runnable)</code>.
   *
   * @param runnable
   *            the runnable to run
   */
  static void Run(ISafeRunnable::Pointer runnable);

};


}

#endif /* BERRYSAFERUNNABLE_H_ */
