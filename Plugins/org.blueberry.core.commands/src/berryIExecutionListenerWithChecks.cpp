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
  if (l == 0) return;

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
  if (l == 0) return;

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
