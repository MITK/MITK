/*=========================================================================

 Program:   BlueBerry Platform
 Language:  C++
 Date:      $Date$
 Version:   $Revision: 15350 $

 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/

#define NOMINMAX
#include "berryInternalJob.h"

#include <Poco/Thread.h>

#include "berryJob.h"
#include "berryJobManager.h"
#include "berryJobExceptions.h"

#include <string>
#include <sstream>
#include <assert.h>
#include <map>

namespace berry
{

InternalJob::InternalJob(std::string name) :
  jobNumber(nextJobNumber++), flags(Job::NONE), name(name), next(0),
      previous(0), priority(Job::LONG), sptr_schedulingRule(0),
      sptr_monitor(0), m_startTime(), waitQueueStamp(T_NONE), ptr_thread(0)
{
  jobEvents.SetExceptionHandler(MessageExceptionHandler<JobListeners>(&ptr_manager->m_JobListeners, &JobListeners::HandleException));
}

JobManager* const InternalJob::ptr_manager = JobManager::GetInstance();

// time_variables definitions 
// implemented as TimeDiff, if needed can be used to create a Poco::Timestamp with the given value 

const Poco::Timestamp::TimeDiff InternalJob::T_INFINITE = std::numeric_limits<Poco::Timestamp::TimeVal>::max();

// indicates if a job has a scheduling time 
// implemented as TimeDiff, if needed can be used to create a Poco::Timestamp with the given value 
const Poco::Timestamp::TimeDiff InternalJob::T_NONE = -1;


void InternalJob::AddLast(InternalJob::Pointer entry)
{
  InternalJob::Pointer last;
  last = this;
  //find the end of the queue
  while (last->previous)
    last = last->previous;
  //add the new entry to the end of the queue
  last->previous = entry.GetPointer();
  entry->next = last;
  entry->previous = 0;
}

bool InternalJob::operator==(const Object* otherJob) const
{
  if (const InternalJob* temp = dynamic_cast<const InternalJob*>(otherJob))
  {
    return temp->m_startTime >= m_startTime;
  }
  else
    return false;
}

const IJobChangeListener::Events& InternalJob::GetListeners() const
{
  return jobEvents;
}


IProgressMonitor::Pointer InternalJob::GetProgressMonitor() const
{
  return sptr_monitor;
}


Poco::Timestamp InternalJob::GetStartTime() const
{
  return m_startTime;
}


int InternalJob::InternalGetState() const
{
  return flags & M_STATE;
}


void InternalJob::InternalSetPriority(int newPriority)
{
  this->priority = newPriority;
}


void InternalJob::InternalSetRule(ISchedulingRule::Pointer rule)
{
  sptr_schedulingRule = rule;
}


//TODO InternalSetState 
void InternalJob::InternalSetState(int i)
{
  flags = (flags & ~M_STATE) | i;
}


bool InternalJob::IsAboutToRunCanceled() const
{
  return (flags & M_ABOUT_TO_RUN_CANCELED) != 0;
}


bool InternalJob::IsRunCanceled() const 
{
	return (flags & M_RUN_CANCELED) != 0;
}

bool InternalJob::IsConflicting(InternalJob::Pointer otherJob) const
{
  ISchedulingRule::Pointer otherRule = otherJob->GetRule();
  if (sptr_schedulingRule.GetPointer() == 0 || otherRule.GetPointer() == 0)
    return false;
  // TODO MultiRule: extend the IsConflicting (...) method with MultiRule
  // if one of the rules is a compound rule, it must be asked the question.
  //if (schedulingRule.GetClass() == MultiRule.class)
  //	return schedulingRule.IsConflicting(otherRule);
  return otherRule->IsConflicting(sptr_schedulingRule);
}


InternalJob::Pointer InternalJob::Next() const
{
  return next;
}


InternalJob::Pointer InternalJob::Previous() const
{
  return InternalJob::Pointer(previous);
}


InternalJob::Pointer InternalJob::Remove()
{
  if (next != 0)
    next->SetPrevious(InternalJob::Pointer(previous));
  if (previous != 0)
    previous->SetNext(next);
  next = previous = 0;
  return InternalJob::Pointer(this);
}


void InternalJob::SetAboutToRunCanceled(bool value) throw (JobRuntimeException)
{
  flags = value ? flags | M_ABOUT_TO_RUN_CANCELED : flags
      & ~M_ABOUT_TO_RUN_CANCELED;
}

void InternalJob::SetRunCanceled(bool value) 
{
	flags = value ? flags | M_RUN_CANCELED : flags & ~M_RUN_CANCELED;
}


void InternalJob::SetNext(InternalJob::Pointer entry)
{
  this->next = entry;
}


void InternalJob::SetPrevious(InternalJob::Pointer entry)
{
  this->previous = entry.GetPointer();
}


void InternalJob::SetProgressMonitor(IProgressMonitor::Pointer monitor)
{
  sptr_monitor = monitor;
}


void InternalJob::SetResult(IStatus::Pointer result) 
{
	m_result = result;
}



void InternalJob::SetStartTime(Poco::Timestamp::TimeDiff time)
{
  m_startTime = m_startTime + time;
}

void InternalJob::SetStartTime(const Poco::Timestamp& newtime)
{
  m_startTime = newtime;
}

std::string InternalJob::ToString()
{
  std::stringstream ss;
  ss << GetName() << "(" << jobNumber << ")";
  return ss.str(); 
}

void InternalJob::SetWaitQueueStamp(Poco::Timestamp waitQueueStamp)
{
  this->waitQueueStamp = waitQueueStamp;
}

Poco::Timestamp InternalJob::GetWaitQueueStamp()
{
  return waitQueueStamp;
}

int InternalJob::nextJobNumber = 0;

void InternalJob::AddJobChangeListener(IJobChangeListener::Pointer listener)
{
  jobEvents.AddListener(listener);
}

bool InternalJob::BelongsTo(Object::Pointer  /*family*/)
{
  return false;
}

bool InternalJob::Cancel()
{
  return ptr_manager->Cancel(InternalJob::Pointer(this));
}

void InternalJob::Canceling()
{
  //default implementation does nothing
}

void InternalJob::Done(IStatus::Pointer endResult)
{
 ptr_manager->EndJob(InternalJob::Pointer(this),endResult, true);
}

std::string InternalJob::GetName() const
{
  return name;
}

int InternalJob::GetPriority() const
{
  return priority;
}


//TODO QualifiedName GetProperty 
//Object::Pointer
//InternalJob
//::GetProperty(QualifiedName key) {
//		// thread safety: (Concurrency001 - copy on write)
//  std::map<QualifiedName, Object::Pointer> temp (properties);
//  if (temp.empty()) return Object::Pointer(0);
//		else return temp[key];
//	}


IStatus::Pointer InternalJob::GetResult() const 
{
  return m_result;
}

ISchedulingRule::Pointer InternalJob::GetRule() const
{
  return sptr_schedulingRule;
}

int InternalJob::GetState() const
{
  int state = flags & M_STATE;
  switch (state)
  {
  //blocked state is equivalent to waiting state for clients
  case BLOCKED:
    return Job::WAITING;
  case ABOUT_TO_RUN:
    return Job::RUNNING;
  case ABOUT_TO_SCHEDULE:
    return Job::WAITING;
  default:
    return state;
  }
}

Poco::Thread*
InternalJob::GetThread() const
{
  return ptr_thread;
}

bool InternalJob::IsSystem() const
{
  return (flags & M_SYSTEM) != 0;
}

bool InternalJob::IsBlocking()
{
  return ptr_manager->IsBlocking(InternalJob::Pointer(this));
}

bool InternalJob::IsUser() const
{
  return (flags & M_USER) != 0;
}


//void 
//InternalJob
//::Join() throws InterruptedException {
//		manager.join(this);
//	}

void InternalJob::RemoveJobChangeListener(IJobChangeListener::Pointer listener)
{
  jobEvents.RemoveListener(listener);
}

void InternalJob::Schedule(Poco::Timestamp::TimeDiff delay)
{
  if (ShouldSchedule())
    ptr_manager->Schedule(InternalJob::Pointer(this), delay, false);
}

void InternalJob::SetName(const std::string& name)
{
  assert(!name.empty());
  this->name = name;
}

void InternalJob::SetPriority(int newPriority)
{
  switch (newPriority)
  {
  case Job::INTERACTIVE:
  case Job::SHORT:
  case Job::LONG:
  case Job::BUILD:
  case Job::DECORATE:
    ptr_manager->SetPriority(InternalJob::Pointer(this), newPriority);
    break;
  default:
    throw IllegalArgumentException(newPriority);
  }
}

void InternalJob::SetProgressGroup(IProgressMonitor::Pointer group, int ticks)
{
  assert(group.GetPointer() != 0);
  InternalJob::Pointer sptr_temp(this);
  IProgressMonitor::Pointer sptr_pm = ptr_manager->CreateMonitor(sptr_temp,
      group, ticks);
  if (sptr_pm != 0)
    SetProgressMonitor(sptr_pm);
}


//TODO QualifiedName SetProperty 
//void
//InternalJob
//::SetProperty(QualifiedName key, Object value) {
//		// thread safety: (Concurrency001 - copy on write)
//		if (value == NULL) {
//			if (properties == NULL)
//				return;
//			ObjectMap temp = (ObjectMap) properties.Clone();
//			temp.Remove(key);
//			if (temp.isEmpty())
//				properties = NULL;
//			else
//				properties = temp;
//		} else {
//			ObjectMap temp = properties;
//			if (temp == NULL)
//				temp = new ObjectMap(5);
//			else
//				temp = (ObjectMap) properties.Clone();
//			temp.Put(key, value);
//			properties = temp;
//		}
//	}

void InternalJob::SetRule(ISchedulingRule::Pointer rule)
{
  ptr_manager->SetRule(InternalJob::Pointer(this), rule);
}

void InternalJob::SetSystem(bool value)
{
  //TODO Error Exception Problem IllegalStateException
  //if (GetState() != Job.NONE)
  //throw  IllegalStateException();
  flags = value ? flags | M_SYSTEM : flags & ~M_SYSTEM;
}

void InternalJob::SetThread(Poco::Thread* thread)
{
  ptr_thread = thread;
}

void InternalJob::SetUser(bool value)
{
  //TODO Error Exception Problem IllegalStateException
  if (GetState() != Job::NONE)
    throw IllegalStateException();
  flags = value ? flags | M_USER : flags & ~M_USER;
}

bool InternalJob::ShouldSchedule()
{
  return true;
}

bool InternalJob::Sleep()
{
  return ptr_manager->Sleep(InternalJob::Pointer(this));
}

void InternalJob::WakeUp(long delay)
{
  ptr_manager->WakeUp(InternalJob::Pointer(this), delay);
}

}
