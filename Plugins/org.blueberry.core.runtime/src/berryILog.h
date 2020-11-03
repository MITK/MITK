/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYILOG_H
#define BERRYILOG_H

template<class T> class QSharedPointer;
class ctkPlugin;

namespace berry {

template<class T> class SmartPointer;
struct ILogListener;
struct IStatus;

/**
 * A log to which status events can be written.  Logs appear on individual
 * plug-ins and on the platform itself.  Clients can register log listeners which
 * will receive notification of all log events as they come in.
 * <p>
 * XXX Need to create a new log interface on common plugin. That interface should be a super interface of this ILog.
 * getBundle() would stay here. In the super interface we would have getName()
 * </p>
 *
 * @noimplement This interface is not intended to be implemented by clients.
 */
struct ILog
{
  virtual ~ILog();

  /**
   * Adds the given log listener to this log.  Subsequently the log listener will
   * receive notification of all log events passing through this log.
   * This method has no effect if the identical listener is already registered on this log.
   *
   * @param listener the listener to add to this log
   * @see Platform#addLogListener(ILogListener)
   */
  virtual void AddLogListener(ILogListener* listener) = 0;

  /**
   * Returns the plug-in with which this log is associated.
   *
   * @return the plug-in with which this log is associated
   */
  virtual QSharedPointer<ctkPlugin> GetBundle() const = 0;

  /**
   * Logs the given status.  The status is distributed to the log listeners
   * installed on this log and then to the log listeners installed on the platform.
   *
   * @param status the status to log
   */
  virtual void Log(const SmartPointer<IStatus>& status) = 0;

  /**
   * Removes the given log listener to this log.  Subsequently the log listener will
   * no longer receive notification of log events passing through this log.
   * This method has no effect if the identical listener is not registered on this log.
   *
   * @param listener the listener to remove
   * @see Platform#removeLogListener(ILogListener)
   */
  virtual void RemoveLogListener(ILogListener* listener) = 0;
};

}

#endif // BERRYILOG_H
