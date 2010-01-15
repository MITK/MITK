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

#ifndef CHERRYJOBLISTENERS_H_
#define CHERRYJOBLISTENERS_H_

#include "cherryJobChangeEvent.h"
#include "../cherryIJobChangeListener.h"

namespace cherry
{

/**
 * Responsible for notifying all job listeners about job lifecycle events.  Uses a
 * specialized iterator to ensure the complex iteration logic is contained in one place.
 */
class JobListeners
{

public:

  struct IListenerDoit
  {
    virtual void Notify(const IJobChangeListener::Events& listener,
        const IJobChangeEvent::ConstPointer event) const = 0;

    virtual ~IListenerDoit() {}
  };

private:

  friend struct JobManager;

  const IListenerDoit* aboutToRun;
  const IListenerDoit* awake;
  const IListenerDoit* done;
  const IListenerDoit* running;
  const IListenerDoit* scheduled;
  const IListenerDoit* sleeping;

  /**
   * The global job listeners.
   */
  IJobChangeListener::Events global;

  /**
   * TODO Could use an instance pool to re-use old event objects
   */
  static JobChangeEvent::Pointer NewEvent(SmartPointer<Job> job);

  static JobChangeEvent::Pointer NewEvent(SmartPointer<Job> job,
      IStatus::Pointer result);

  static JobChangeEvent::Pointer NewEvent(SmartPointer<Job> job, Poco::Timestamp::TimeDiff delay);


  /**
   * Process the given doit for all global listeners and all local listeners
   * on the given job.
   */
  void DoNotify(const IListenerDoit* doit,
      const IJobChangeEvent::ConstPointer event);


public:

  JobListeners();
  ~JobListeners();

  void HandleException(const std::exception& e);

  void Add(IJobChangeListener::Pointer listener)
  {
    global.AddListener(listener);
  }

  void Remove(IJobChangeListener::Pointer listener)
  {
    global.RemoveListener(listener);
  }

  void AboutToRun(SmartPointer<Job> job)
  {
    DoNotify(aboutToRun, NewEvent(job));
  }

  void Awake(SmartPointer<Job> job)
  {
    DoNotify(awake, NewEvent(job));
  }

  void Done(SmartPointer<Job> job, IStatus::Pointer result, bool reschedule)
  {
    JobChangeEvent::Pointer event = NewEvent(job, result);
    event->reschedule = reschedule;
    DoNotify(done, event);
  }

  void Running(SmartPointer<Job> job)
  {
    DoNotify(running, NewEvent(job));
  }

  void Scheduled(SmartPointer<Job> job, Poco::Timestamp::TimeDiff delay, bool reschedule)
  {
    JobChangeEvent::Pointer event = NewEvent(job, delay);
    event->reschedule = reschedule;
    DoNotify(scheduled, event);
  }

  void Sleeping(SmartPointer<Job> job)
  {
    DoNotify(sleeping, NewEvent(job));
  }

};

}

#endif /* CHERRYJOBLISTENERS_H_ */
