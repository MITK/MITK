/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryIExecutionListenerWithChecks.h"

#include "berryExecutionEvent.h"
#include "berryIParameterValueConverter.h"
#include "berryCommandCategory.h"
#include "berryState.h"
#include "berryIHandler.h"

namespace berry {

void
IExecutionListenerWithChecks::Events
::AddListener(IExecutionListener* l)
{
  if (l == nullptr) return;

  IExecutionListener::Events::AddListener(l);

  if (IExecutionListenerWithChecks* cl = dynamic_cast<IExecutionListenerWithChecks*>(l))
  {
    notDefined += NotDefinedDelegate(cl, &IExecutionListenerWithChecks::NotDefined);
    notEnabled += NotEnabledDelegate(cl, &IExecutionListenerWithChecks::NotEnabled);
  }
}

void
IExecutionListenerWithChecks::Events
::RemoveListener(IExecutionListener* l)
{
  if (l == nullptr) return;

  IExecutionListener::Events::RemoveListener(l);

  if (IExecutionListenerWithChecks* cl = dynamic_cast<IExecutionListenerWithChecks*>(l))
  {
    notDefined -= NotDefinedDelegate(cl, &IExecutionListenerWithChecks::NotDefined);
    notEnabled -= NotEnabledDelegate(cl, &IExecutionListenerWithChecks::NotEnabled);
  }
}

bool
IExecutionListenerWithChecks::Events
::HasListeners() const
{
  return IExecutionListener::Events::HasListeners() || notDefined.HasListeners() ||
         notEnabled.HasListeners();
}

bool
IExecutionListenerWithChecks::Events
::IsEmpty() const
{
  return !this->HasListeners();
}

}
