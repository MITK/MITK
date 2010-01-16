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

#include "berryJobListeners.h"
#include "berryJobManager.h"

#include "../berryJob.h"

#include <berrySolsticeExceptions.h>
#include <typeinfo>

namespace berry
{

struct AboutToRunDoit: public JobListeners::IListenerDoit
{
  void Notify(const IJobChangeListener::Events& events,
      const IJobChangeEvent::ConstPointer event) const
  {
    events.jobAboutToRun(event);
  }
};

struct AwakeDoit: public JobListeners::IListenerDoit
{
  void Notify(const IJobChangeListener::Events& events,
      const IJobChangeEvent::ConstPointer event) const
  {
    events.jobAwake(event);
  }
};

struct DoneDoit: public JobListeners::IListenerDoit
{
  void Notify(const IJobChangeListener::Events& events,
      const IJobChangeEvent::ConstPointer event) const
  {
    events.jobDone(event);
  }
};

struct RunningDoit: public JobListeners::IListenerDoit
{
  void Notify(const IJobChangeListener::Events& events,
      const IJobChangeEvent::ConstPointer event) const
  {
    events.jobRunning(event);
  }
};

struct ScheduledDoit: public JobListeners::IListenerDoit
{
  void Notify(const IJobChangeListener::Events& events,
      const IJobChangeEvent::ConstPointer event) const
  {
    events.jobScheduled(event);
  }
};

struct SleepingDoit: public JobListeners::IListenerDoit
{
  void Notify(const IJobChangeListener::Events& events,
      const IJobChangeEvent::ConstPointer event) const
  {
    events.jobSleeping(event);
  }
};

JobListeners::JobListeners() :
  aboutToRun(new AboutToRunDoit()), awake(new AwakeDoit()),
      done(new DoneDoit()), running(new RunningDoit()), scheduled(
          new ScheduledDoit()), sleeping(new SleepingDoit())
{

}

JobListeners::~JobListeners()
{
  delete aboutToRun;
  delete awake;
  delete done;
  delete running;
  delete scheduled;
  delete sleeping;
}

JobChangeEvent::Pointer JobListeners::NewEvent(Job::Pointer job)
{
  JobChangeEvent::Pointer instance(new JobChangeEvent());
  instance->job = job;
  return instance;
}

JobChangeEvent::Pointer JobListeners::NewEvent(Job::Pointer job, IStatus::Pointer result)
{
  JobChangeEvent::Pointer instance(new JobChangeEvent());
  instance->job = job;
  instance->result = result;
  return instance;
}

JobChangeEvent::Pointer JobListeners::NewEvent(Job::Pointer job, Poco::Timestamp::TimeDiff delay)
{
  JobChangeEvent::Pointer instance(new JobChangeEvent());
  instance->job = job;
  instance->delay = delay;
  return instance;
}

void JobListeners::DoNotify(const IListenerDoit* doit,
    const IJobChangeEvent::ConstPointer event)
{
  //notify all global listeners
  doit->Notify(global, event);

  //notify all local listeners
  const IJobChangeListener::Events& events =
      event->GetJob().Cast<InternalJob> ()->GetListeners();
  doit->Notify(events, event);
}

void JobListeners::HandleException(const std::exception& e)
{
  //this code is roughly copied from InternalPlatform.run(ISafeRunnable),
  //but in-lined here for performance reasons
  try
  {
    dynamic_cast<const OperationCanceledException&> (e);
    return;
  } catch (const std::bad_cast&)
  {
    // TODO get bundle id (find a C++ way)
    //std::string pluginId = JobOSGiUtils.getDefault().getBundleId(listener);
    std::string pluginId;
    if (pluginId.empty())
      pluginId = JobManager::PI_JOBS;
    std::string message = "Problems occurred when invoking code from plug-in: "
        + pluginId;
    std::cerr << message << std::endl;
    // TODO Logging
    //  RuntimeLog.log(new Status(IStatus.ERROR, pluginId, JobManager.PLUGIN_ERROR,
    //      message, e));
  }
}

}
