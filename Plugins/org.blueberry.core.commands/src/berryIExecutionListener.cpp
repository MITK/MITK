/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
  if (l == nullptr) return;

  notHandled += NotHandledDelegate(l, &IExecutionListener::NotHandled);
  postExecuteFailure += PostExecuteFailureDelegate(l, &IExecutionListener::PostExecuteFailure);
  postExecuteSuccess += PostExecuteSuccessDelegate(l, &IExecutionListener::PostExecuteSuccess);
  preExecute += PreExecuteDelegate(l, &IExecutionListener::PreExecute);
}

void
IExecutionListener::Events
::RemoveListener(IExecutionListener* l)
{
  if (l == nullptr) return;

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




