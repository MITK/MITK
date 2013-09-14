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

#ifndef BERRYISAFERUNNABLE_H_
#define BERRYISAFERUNNABLE_H_

#include <berryObject.h>
#include <berryMacros.h>

#include <org_blueberry_solstice_common_Export.h>

namespace berry
{

/**
 * Safe runnables represent blocks of code and associated exception
 * handlers.  They are typically used when a plug-in needs to call some
 * untrusted code (e.g., code contributed by another plug-in via an
 * extension).
 * <p>
 * This interface can be used without OSGi running.
 * </p><p>
 * Clients may implement this interface.
 * </p>
 * @see SafeRunner#run(ISafeRunnable)
 */
struct BERRY_COMMON_RUNTIME ISafeRunnable: public Object
{

berryInterfaceMacro(ISafeRunnable, berry);

  /**
   * Handles an exception thrown by this runnable's <code>run</code>
   * method.  The processing done here should be specific to the
   * particular usecase for this runnable.  Generalized exception
   * processing (e.g., logging in the platform's log) is done by the
   * Platform's run mechanism.
   *
   * @param exception an exception which occurred during processing
   *    the body of this runnable (i.e., in <code>run()</code>)
   * @see SafeRunner#run(ISafeRunnable)
   */
  virtual void HandleException(const std::exception& exception) = 0;

  /**
   * Runs this runnable.  Any exceptions thrown from this method will
   * be passed to this runnable's <code>handleException</code>
   * method.
   *
   * @exception Exception if a problem occurred while running this method.
   *    The exception will be processed by <code>handleException</code>
   * @see SafeRunner#run(ISafeRunnable)
   */
  virtual void Run() = 0;
};

template<typename R>
struct SafeRunnableDelegate: public ISafeRunnable
{
  typedef void(R::*RunCallback)();
  typedef void(R::*HandleExcCallback)(const std::exception&);

  SafeRunnableDelegate(R* runnable, RunCallback func, HandleExcCallback handleFunc = 0) :
    m_Runnable(runnable), m_RunFunc(func), m_HandleExcFunc(handleFunc)
  {
  }

  void Run()
  {
    m_Runnable->*m_RunFunc();
  }

  void HandleException(const std::exception& exception)
  {
    if (m_HandleExcFunc)
      m_Runnable->*m_HandleExcFunc(exception);
  }

private:

  R* m_Runnable;
  RunCallback m_RunFunc;
  HandleExcCallback m_HandleExcFunc;

};

}

#endif /* BERRYISAFERUNNABLE_H_ */
