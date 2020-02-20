/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryISaveablesLifecycleListener.h"

namespace berry {

void
ISaveablesLifecycleListener::Events
::AddListener(ISaveablesLifecycleListener* listener)
{
  if (listener == nullptr) return;

  this->lifecycleChange += Delegate(listener, &ISaveablesLifecycleListener::HandleLifecycleEvent);
}

void
ISaveablesLifecycleListener::Events
::RemoveListener(ISaveablesLifecycleListener* listener)
{
  if (listener == nullptr) return;

  this->lifecycleChange -= Delegate(listener, &ISaveablesLifecycleListener::HandleLifecycleEvent);
}

ISaveablesLifecycleListener::~ISaveablesLifecycleListener()
{
}

}
