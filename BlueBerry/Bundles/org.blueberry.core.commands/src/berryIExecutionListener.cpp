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

#include "berryIExecutionListener.h"

#include "berryIParameterValueConverter.h"
#include "berryExecutionEvent.h"
#include "berryCommandCategory.h"
#include "berryIHandler.h"

namespace berry {
  
IExecutionListener::Events::~Events()
{
}
  
void
IExecutionListener::Events
::AddListener(IExecutionListener::Pointer l)
{
  if (l.IsNull()) return;

  notHandled += NotHandledDelegate(l.GetPointer(), &IExecutionListener::NotHandled);
  postExecuteFailure += PostExecuteFailureDelegate(l.GetPointer(), &IExecutionListener::PostExecuteFailure);
  postExecuteSuccess += PostExecuteSuccessDelegate(l.GetPointer(), &IExecutionListener::PostExecuteSuccess);
  preExecute += PreExecuteDelegate(l.GetPointer(), &IExecutionListener::PreExecute);
}

void
IExecutionListener::Events
::RemoveListener(IExecutionListener::Pointer l)
{
  if (l.IsNull()) return;

  notHandled -= NotHandledDelegate(l.GetPointer(), &IExecutionListener::NotHandled);
  postExecuteFailure -= PostExecuteFailureDelegate(l.GetPointer(), &IExecutionListener::PostExecuteFailure);
  postExecuteSuccess -= PostExecuteSuccessDelegate(l.GetPointer(), &IExecutionListener::PostExecuteSuccess);
  preExecute -= PreExecuteDelegate(l.GetPointer(), &IExecutionListener::PreExecute);
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




