/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryJob.h"
#include "berryIJobManager.h"
#include "internal/berryJobManager.h"
#include "berryIStatus.h"
#include "berryStatus.h"

#include <string>

namespace berry
{



const IStatus::Pointer Job::ASYNC_FINISH(
    new Status (IStatus::OK_TYPE, JobManager::PI_JOBS(),
                1,
                "",
                berry::Status::SourceLocation(__FILE__, "berry::Job::ASYNC_FINISH", __LINE__)
                ));

Job::Job(const QString& name)
  : InternalJob(name)
{
}

const IJobManager* Job::GetJobManager()
{
  return ptr_manager;
}


bool Job::BelongsTo(Object::Pointer  /*family*/)
{
  return false;
}

bool Job::Cancel()
{
  return InternalJob::Cancel();
}

void Job::Done(IStatus::Pointer result)
{
  InternalJob::Done(result);
}

QString Job::GetName() const
{
  return InternalJob::GetName();
}

int Job::GetPriority() const
{
  return InternalJob::GetPriority();
}

// TODO QualifiedName muss noch implementiert werden
// Object Job::GetProperty(QualifiedName key)
// {
//    return InternalJob::GetProperty(key);
//  }


IStatus::Pointer Job::GetResult() const
 {
    return InternalJob::GetResult();
  }

ISchedulingRule::Pointer Job::GetRule() const
{
  return InternalJob::GetRule();
}

int Job::GetState() const
{
  return InternalJob::GetState();
}

Poco::Thread*
Job::GetThread() const
{
  return InternalJob::GetThread();
}

bool Job::IsBlocking()
{
  return InternalJob::IsBlocking();
}

bool Job::IsSystem() const
{
  return InternalJob::IsSystem();
}

bool Job::IsUser() const
{
  return InternalJob::IsUser();
}

// TODO Join
//void Job::Join()
// throw (InterruptedException)
// {
//    InternalJob::Join();
//  }

void Job::RemoveJobChangeListener(IJobChangeListener* listener)
{
  InternalJob::RemoveJobChangeListener(listener);
}

void Job::Schedule()
{
  Poco::Timestamp::TimeDiff tmpNoDelay = 0;
  InternalJob::Schedule(tmpNoDelay);
}

void Job::Schedule(Poco::Timestamp::TimeDiff delay)
{
  InternalJob::Schedule(delay);
}

void Job::SetName(const QString& name)
{
  InternalJob::SetName(name);
}

void Job::SetPriority(int priority)
{
  InternalJob::SetPriority(priority);
}

void Job::SetProgressGroup(IProgressMonitor::Pointer group, int ticks)
{
  InternalJob::SetProgressGroup(group, ticks);
}

// TODO SetProperty
// void Job::SetProperty(QualifiedName key, Object value)
// {
//    InternalJob::SetProperty(key, value);
//  }

void Job::SetRule(ISchedulingRule::Pointer rule)
{
  InternalJob::SetRule(rule);
}

void Job::SetSystem(bool value)
{
  InternalJob::SetSystem(value);
}

void Job::SetUser(bool value)
{
  InternalJob::SetUser(value);
}

void Job::SetThread(Poco::Thread* thread)
{
  InternalJob::SetThread(thread);
}

bool Job::ShouldRun()
{
  return true;
}

bool Job::ShouldSchedule()
{
  return true;
}

bool Job::Sleep()
{
  return InternalJob::Sleep();
}

void Job::WakeUp()
{
  InternalJob::WakeUp(0);
}

void Job::WakeUp(long delay)
{
  InternalJob::WakeUp(delay);
}

void Job::Canceling()
{
  //default implementation does nothing
}

}
