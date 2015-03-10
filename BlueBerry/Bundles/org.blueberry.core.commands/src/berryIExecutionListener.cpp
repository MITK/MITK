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

#include "berryIExecutionListener.h"

#include "berryIParameterValueConverter.h"
#include "berryExecutionEvent.h"
#include "berryCommandCategory.h"
#include "berryState.h"
#include "berryIHandler.h"

namespace berry {

IExecutionListener::Events::~Events()
{
}

void
IExecutionListener::Events
::AddListener(IExecutionListener* l)
{
  if (l == 0) return;

  notHandled += NotHandledDelegate(l, &IExecutionListener::NotHandled);
  postExecuteFailure += PostExecuteFailureDelegate(l, &IExecutionListener::PostExecuteFailure);
  postExecuteSuccess += PostExecuteSuccessDelegate(l, &IExecutionListener::PostExecuteSuccess);
  preExecute += PreExecuteDelegate(l, &IExecutionListener::PreExecute);
}

void
IExecutionListener::Events
::RemoveListener(IExecutionListener* l)
{
  if (l == 0) return;

  notHandled -= NotHandledDelegate(l, &IExecutionListener::NotHandled);
  postExecuteFailure -= PostExecuteFailureDelegate(l, &IExecutionListener::PostExecuteFailure);
  postExecuteSuccess -= PostExecuteSuccessDelegate(l, &IExecutionListener::PostExecuteSuccess);
  preExecute -= PreExecuteDelegate(l, &IExecutionListener::PreExecute);
}

bool
IExecutionListener::Events
::HasListeners() const
{
  return notHandled.HasListeners() || postExecuteFailure.HasListeners() ||
         postExecuteSuccess.HasListeners() || preExecute.HasListeners();
}

bool
IExecutionListener::Events
::IsEmpty() const
{
  return !this->HasListeners();
}

IExecutionListener::~IExecutionListener()
{
}

}




