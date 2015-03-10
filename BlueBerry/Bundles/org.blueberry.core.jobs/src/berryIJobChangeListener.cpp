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

#include "berryIJobChangeListener.h"

namespace berry
{

void IJobChangeListener::Events::AddListener(IJobChangeListener* listener)
{
  if (!listener)
    return;

  Types types = listener->GetEventTypes();

  if (types & ABOUT_TO_RUN)
    jobAboutToRun += Delegate(listener, &IJobChangeListener::AboutToRun);
  if (types & AWAKE)
    jobAwake += Delegate(listener, &IJobChangeListener::Awake);
  if (types & DONE)
    jobDone += Delegate(listener, &IJobChangeListener::Done);
  if (types & RUNNING)
    jobRunning += Delegate(listener, &IJobChangeListener::Running);
  if (types & SCHEDULED)
    jobScheduled += Delegate(listener, &IJobChangeListener::Scheduled);
  if (types & SLEEPING)
    jobSleeping += Delegate(listener, &IJobChangeListener::Sleeping);
}

void IJobChangeListener::Events::RemoveListener(IJobChangeListener* listener)
{
  if (!listener)
    return;

  jobAboutToRun -= Delegate(listener, &IJobChangeListener::AboutToRun);
  jobAwake -= Delegate(listener, &IJobChangeListener::Awake);
  jobDone -= Delegate(listener, &IJobChangeListener::Done);
  jobRunning -= Delegate(listener, &IJobChangeListener::Running);
  jobScheduled -= Delegate(listener, &IJobChangeListener::Scheduled);
  jobSleeping -= Delegate(listener, &IJobChangeListener::Sleeping);
}

void IJobChangeListener::Events::SetExceptionHandler(const AbstractExceptionHandler& handler)
{
  jobAboutToRun.SetExceptionHandler(handler);
  jobAwake.SetExceptionHandler(handler);
  jobDone.SetExceptionHandler(handler);
  jobRunning.SetExceptionHandler(handler);
  jobScheduled.SetExceptionHandler(handler);
  jobSleeping.SetExceptionHandler(handler);
}

}
