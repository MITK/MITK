/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryJobListeners.h"
#include "berryJobManager.h"

#include "berryJob.h"
#include "berryOperationCanceledException.h"
#include "berryLog.h"

#include <typeinfo>

namespace berry
{

struct AboutToRunDoit: public JobListeners::IListenerDoit
{
  void Notify(const IJobChangeListener::Events& events,
      const IJobChangeEvent::ConstPointer& event) const override
  {
    events.jobAboutToRun(event);
  }
};

struct AwakeDoit: public JobListeners::IListenerDoit
{
  void Notify(const IJobChangeListener::Events& events,
      const IJobChangeEvent::ConstPointer& event) const override
  {
    events.jobAwake(event);
  }
};

struct DoneDoit: public JobListeners::IListenerDoit
{
  void Notify(const IJobChangeListener::Events& events,
      const IJobChangeEvent::ConstPointer& event) const override
  {
    events.jobDone(event);
  }
};

struct RunningDoit: public JobListeners::IListenerDoit
{
  void Notify(const IJobChangeListener::Events& events,
      const IJobChangeEvent::ConstPointer& event) const override
  {
    events.jobRunning(event);
  }
};

struct ScheduledDoit: public JobListeners::IListenerDoit
{
  void Notify(const IJobChangeListener::Events& events,
      const IJobChangeEvent::ConstPointer& event) const override
  {
    events.jobScheduled(event);
  }
};

struct SleepingDoit: public JobListeners::IListenerDoit
{
  void Notify(const IJobChangeListener::Events& events,
      const IJobChangeEvent::ConstPointer& event) const override
  {
    events.jobSleeping(event);
  }
};

JobListeners::JobListeners()
  : aboutToRun(new AboutToRunDoit())
  , awake(new AwakeDoit())
  , done(new DoneDoit())
  , running(new RunningDoit())
  , scheduled(new ScheduledDoit())
  , sleeping(new SleepingDoit())
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
    const IJobChangeEvent::ConstPointer& event)
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
  if(dynamic_cast<const OperationCanceledException*>(&e))
  {
    return;
  }
  else
  {
    // TODO get bundle id (find a C++ way)
    //std::string pluginId = JobOSGiUtils.getDefault().getBundleId(listener);
    QString pluginId;
    if (pluginId.isEmpty())
    {
      pluginId = JobManager::PI_JOBS();
    }
    QString message = "Problems occurred when invoking code from plug-in: "
                      + pluginId;
    BERRY_ERROR << message;
    // TODO Logging
    //  RuntimeLog.log(new Status(IStatus.ERROR, pluginId, JobManager.PLUGIN_ERROR,
    //      message, e));
  }
}

}
