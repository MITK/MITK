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

#ifndef BERRYIJOBCHANGELISTENER_H_
#define BERRYIJOBCHANGELISTENER_H_

#include "berryIJobChangeEvent.h"

namespace berry
{

/**
 * Callback interface for clients interested in being notified when jobs change state.
 * <p>
 * A single job listener instance can be added either to the job manager, for notification
 * of all scheduled jobs, or to any set of individual jobs.  A single listener instance should
 * not be added to both the job manager, and to individual jobs (such a listener may
 * receive duplicate notifications).
 * </p><p>
 * Clients should not rely on the result of the <code>Job#GetState()</code>
 * method on jobs for which notification is occurring. Listeners are notified of 
 * all job state changes, but whether the state change occurs before, during, or 
 * after listeners are notified is unspecified.
 * </p><p>
 * Clients may implement this interface.
 * </p>
 * @see IJobManager#AddJobChangeListener(IJobChangeListener::Pointer)
 * @see IJobManager#RemoveJobChangeListener(IJobChangeListener::Pointer)
 * @see Job#AddJobChangeListener(IJobChangeListener::Pointer)
 * @see Job#GetState()
 * @see Job#RemoveJobChangeListener(IJobChangeListener::Pointer)
 */

struct BERRY_JOBS IJobChangeListener: public Object
{

berryInterfaceMacro(IJobChangeListener, berry)

  struct BERRY_JOBS Events
  {

    typedef Message1<const IJobChangeEvent::ConstPointer> JobChangeEventType;

    enum Type
    {
      NONE = 0x00000000,
      ABOUT_TO_RUN = 0x00000001,
      AWAKE = 0x00000002,
      DONE = 0x00000004,
      RUNNING = 0x00000008,
      SCHEDULED = 0x00000010,
      SLEEPING = 0x00000020,

      ALL = 0xffffffff
    };

    BERRY_DECLARE_FLAGS(Types, Type)

    JobChangeEventType jobAboutToRun;
    JobChangeEventType jobAwake;
    JobChangeEventType jobDone;
    JobChangeEventType jobRunning;
    JobChangeEventType jobScheduled;
    JobChangeEventType jobSleeping;

    void AddListener(IJobChangeListener::Pointer listener);
    void RemoveListener(IJobChangeListener::Pointer listener);

    void SetExceptionHandler(const AbstractExceptionHandler& handler);

    typedef MessageDelegate1<IJobChangeListener, const IJobChangeEvent::ConstPointer>
        Delegate;
  };

  virtual Events::Types GetEventTypes() = 0;

  /**
   * Notification that a job is about to be run. Listeners are allowed to sleep, cancel,
   * or change the priority of the job before it is started (and as a result may prevent
   * the run from actually occurring).
   *
   * @param event the event details
   */
  virtual void AboutToRun(const IJobChangeEvent::ConstPointer /*event*/)
  {
  }
  ;

  /**
   * Notification that a job was previously sleeping and has now been rescheduled
   * to run.
   *
   * @param event the event details
   */
  virtual void Awake(const IJobChangeEvent::ConstPointer /*event*/)
  {
  }
  ;

  /**
   * Notification that a job has completed execution, either due to cancelation, successful
   * completion, or failure.  The event status object indicates how the job finished,
   * and the reason for failure, if applicable.
   *
   * @param event the event details
   */
  virtual void Done(const IJobChangeEvent::ConstPointer /*event*/)
  {
  }
  ;

  /**
   * Notification that a job has started running.
   *
   * @param event the event details
   */
  virtual void Running(const IJobChangeEvent::ConstPointer /*event*/)
  {
  }
  ;

  /**
   * Notification that a job is being added to the queue of scheduled jobs.
   * The event details includes the scheduling delay before the job should start
   * running.
   *
   * @param event the event details, including the job instance and the scheduling
   * delay
   */
  virtual void Scheduled(const IJobChangeEvent::ConstPointer /*event*/)
  {
  }
  ;

  /**
   * Notification that a job was waiting to run and has now been put in the
   * sleeping state.
   *
   * @param event the event details
   */
  virtual void Sleeping(const IJobChangeEvent::ConstPointer /*event*/)
  {
  }
  ;

};

}

BERRY_DECLARE_OPERATORS_FOR_FLAGS(berry::IJobChangeListener::Events::Types)

#endif /* BERRYIJOBCHANGELISTENER_H_ */
