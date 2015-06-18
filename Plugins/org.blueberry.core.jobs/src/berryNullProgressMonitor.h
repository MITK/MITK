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

#ifndef BERRY_NULLPROGRESSMONITOR_H
#define BERRY_NULLPROGRESSMONITOR_H

#include <string>
#include "berryIProgressMonitor.h"

namespace berry
{

/**
 * A default progress monitor implementation suitable for
 * subclassing.
 * <p>
 * This implementation supports cancellation. The default
 * implementations of the other methods do nothing.
 * </p><p>
 * This class can be used without OSGi running.
 * </p>
 */
class NullProgressMonitor: public IProgressMonitor
{

public:

  berryObjectMacro(NullProgressMonitor);

  /**
   * Constructs a new progress monitor.
   */
  NullProgressMonitor();

  /**
   * This implementation does nothing.
   * Subclasses may override this method to do interesting
   * processing when a task begins.
   *
   * @see IProgressMonitor#BeginTask(std::string, int)
   */
  virtual void BeginTask(const std::string& name, int totalWork) override;

  /**
   * This implementation does nothing.
   * Subclasses may override this method to do interesting
   * processing when a task is done.
   *
   * @see IProgressMonitor#Done()
   */
  virtual void Done() override;

  /**
   * This implementation does nothing.
   * Subclasses may override this method.
   *
   * @see IProgressMonitor#InternalWorked(double)
   */
  virtual void InternalWorked(double work) override;

  /**
   * This implementation returns the value of the internal
   * state variable set by <code>setCanceled</code>.
   * Subclasses which override this method should
   * override <code>setCanceled</code> as well.
   *
   * @see IProgressMonitor#IsCanceled()
   * @see IProgressMonitor#SetCanceled(bool)
   */
  virtual bool IsCanceled() override;

  /**
   * This implementation sets the value of an internal state variable.
   * Subclasses which override this method should override
   * <code>isCanceled</code> as well.
   *
   * @see IProgressMonitor#IsCanceled()
   * @see IProgressMonitor#SetCanceled(bool)
   */
  virtual void SetCanceled(bool cancelled) override;

  /**
   * This implementation does nothing.
   * Subclasses may override this method to do something
   * with the name of the task.
   *
   * @see IProgressMonitor#SetTaskName(const std::string&)
   */
  virtual void SetTaskName(const std::string& name) override;

  /**
   * This implementation does nothing.
   * Subclasses may override this method to do interesting
   * processing when a subtask begins.
   *
   * @see IProgressMonitor#SubTask(const std::string&)
   */
  virtual void SubTask(const std::string& name) override;

  /**
   * This implementation does nothing.
   * Subclasses may override this method to do interesting
   * processing when some work has been completed.
   *
   * @see IProgressMonitor#Worked(int)
   */
  virtual void Worked(int work) override;

private:

  /**
   * Indicates whether cancel has been requested.
   */
  bool m_cancelled;

};

}

#endif /* BERRY_NULLPROGRESSMONITOR_H  */
