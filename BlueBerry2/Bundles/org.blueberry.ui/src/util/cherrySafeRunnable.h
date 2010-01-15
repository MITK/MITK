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


#ifndef CHERRYSAFERUNNABLE_H_
#define CHERRYSAFERUNNABLE_H_

#include <cherryISafeRunnable.h>
#include "cherryISafeRunnableRunner.h"

namespace cherry {

/**
 * Implements a default implementation of ISafeRunnable. The default
 * implementation of <code>HandleException</code> opens a dialog to show any
 * errors as they accumulate.
 * <p>
 * This may be executed on any thread.
 */
class CHERRY_UI SafeRunnable : public ISafeRunnable {

private:

  static bool ignoreErrors;

  static ISafeRunnableRunner::Pointer runner;

  std::string message;

   /**
   * Creates the default safe runnable runner.
   *
   * @return the default safe runnable runner
   */
  static ISafeRunnableRunner::Pointer CreateDefaultRunner();

public:

  /**
   * Creates a new instance of SafeRunnable with the given error message.
   *
   * @param message
   *            the error message to use
   */
  SafeRunnable(const std::string& message = "");

  /*
   * (non-Javadoc)
   *
   * @see ISafeRunnable#HandleException(const std::exception&)
   */
  void HandleException(const std::exception& e);

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

#endif /* CHERRYSAFERUNNABLE_H_ */
