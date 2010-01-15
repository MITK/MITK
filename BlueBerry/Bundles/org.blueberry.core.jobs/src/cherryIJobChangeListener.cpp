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

#include "cherryIJobChangeListener.h"

namespace cherry
{

void IJobChangeListener::Events::AddListener(
    IJobChangeListener::Pointer listener)
{
  if (!listener)
    return;

  Types types = listener->GetEventTypes();

  if (types & ABOUT_TO_RUN)
    jobAboutToRun += Delegate(listener.GetPointer(),
        &IJobChangeListener::AboutToRun);
  if (types & AWAKE)
    jobAwake += Delegate(listener.GetPointer(), &IJobChangeListener::Awake);
  if (types & DONE)
    jobDone += Delegate(listener.GetPointer(), &IJobChangeListener::Done);
  if (types & RUNNING)
    jobRunning += Delegate(listener.GetPointer(), &IJobChangeListener::Running);
  if (types & SCHEDULED)
    jobScheduled += Delegate(listener.GetPointer(),
        &IJobChangeListener::Scheduled);
  if (types & SLEEPING)
    jobSleeping += Delegate(listener.GetPointer(),
        &IJobChangeListener::Sleeping);
}

void IJobChangeListener::Events::RemoveListener(
    IJobChangeListener::Pointer listener)
{
  if (!listener)
    return;

  jobAboutToRun -= Delegate(listener.GetPointer(),
      &IJobChangeListener::AboutToRun);
  jobAwake -= Delegate(listener.GetPointer(), &IJobChangeListener::Awake);
  jobDone -= Delegate(listener.GetPointer(), &IJobChangeListener::Done);
  jobRunning -= Delegate(listener.GetPointer(), &IJobChangeListener::Running);
  jobScheduled -= Delegate(listener.GetPointer(),
      &IJobChangeListener::Scheduled);
  jobSleeping -= Delegate(listener.GetPointer(), &IJobChangeListener::Sleeping);
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
