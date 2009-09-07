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

#ifndef CHERRYISAFERUNNABLE_H_
#define CHERRYISAFERUNNABLE_H_

#include <cherryObject.h>
#include <cherryMacros.h>

namespace cherry
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
struct ISafeRunnable: public Object
{

cherryInterfaceMacro(ISafeRunnable, cherry)

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
  typedef void(R::*CallbackFunc)();

  SafeRunnableDelegate(R* runnable, CallbackFunc func) :
    m_Runnable(runnable), m_Func(func)
  {
  }

  void Run()
  {
    m_Runnable->*m_Func();
  }

private:

  R* m_Runnable;
  CallbackFunc m_Func;

};

}

#endif /* CHERRYISAFERUNNABLE_H_ */
