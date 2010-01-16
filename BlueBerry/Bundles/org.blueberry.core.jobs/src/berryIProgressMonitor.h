/*=========================================================================

 Program:   BlueBerry Platform
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

#ifndef  BERRY_IPROGRESS_MONITOR_H
#define  BERRY_IPROGRESS_MONITOR_H

#include "berryJobsDll.h"
#include "berryObject.h"
#include <string>

namespace berry
{

struct IProgressMonitor: public Object
{

berryInterfaceMacro(IProgressMonitor, berry)

  /** Constant indicating an unknown amount of work. */
    
  static const int UNKNOWN = -1;

  /**
   * Notifies that the main task is beginning.  This must only be called once
   * on a given progress monitor instance.
   *
   * @param name the name (or description) of the main task
   * @param totalWork the total number of work units into which
   *  the main task is been subdivided. If the value is <code>UNKNOWN</code>
   *  the implementation is free to indicate progress in a way which
   *  doesn't require the total number of work units in advance.
   */
  virtual void BeginTask(const std::string& name, int totalWork) = 0;

  /**
   * Notifies that the work is done; that is, either the main task is completed
   * or the user canceled it. This method may be called more than once
   * (implementations should be prepared to handle this case).
   */
  virtual void Done() = 0;

  /**
   * Internal method to handle scaling correctly. This method
   * must not be called by a client. Clients should
   * always use the method </code>worked(int)</code>.
   *
   * @param work the amount of work done
   */
  virtual void InternalWorked(double work) = 0;

  /**
   * Returns whether cancellation of current operation has been requested.
   * Long-running operations should poll to see if cancellation
   * has been requested.
   *
   * @return <code>true</code> if cancellation has been requested,
   *    and <code>false</code> otherwise
   * @see #setCanceled(bool)
   */
  virtual bool IsCanceled() = 0;

  /**
   * Sets the cancel state to the given value.
   *
   * @param value <code>true</code> indicates that cancellation has
   *     been requested (but not necessarily acknowledged);
   *     <code>false</code> clears this flag
   * @see #isCanceled()
   */
  virtual void SetCanceled(bool value) = 0;

  /**
   * Sets the task name to the given value. This method is used to
   * restore the task label after a nested operation was executed.
   * Normally there is no need for clients to call this method.
   *
   * @param name the name (or description) of the main task
   * @see #beginTask
   */
  virtual void SetTaskName(const std::string& name) = 0;

  /**
   * Notifies that a subtask of the main task is beginning.
   * Subtasks are optional; the main task might not have subtasks.
   *
   * @param name the name (or description) of the subtask
   */
  virtual void SubTask(const std::string& name) = 0;

  /**
   * Notifies that a given number of work unit of the main task
   * has been completed. Note that this amount represents an
   * installment, as opposed to a cumulative amount of work done
   * to date.
   *
   * @param work a non-negative number of work units just completed
   */
  virtual void Worked(int work) = 0;

};

}

#endif /* _BERRY_IPROGRESS_MONITOR_H */
