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

#include "berryISaveablesLifecycleListener.h"

namespace berry {

void
ISaveablesLifecycleListener::Events
::AddListener(ISaveablesLifecycleListener* listener)
{
  if (listener == NULL) return;

  this->lifecycleChange += Delegate(listener, &ISaveablesLifecycleListener::HandleLifecycleEvent);
}

void
ISaveablesLifecycleListener::Events
::RemoveListener(ISaveablesLifecycleListener* listener)
{
  if (listener == NULL) return;

  this->lifecycleChange -= Delegate(listener, &ISaveablesLifecycleListener::HandleLifecycleEvent);
}

ISaveablesLifecycleListener::~ISaveablesLifecycleListener()
{
}

}
