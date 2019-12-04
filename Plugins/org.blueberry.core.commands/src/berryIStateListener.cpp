/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryIStateListener.h"

#include "berryState.h"

namespace berry {

void
IStateListener::Events
::AddListener(IStateListener* l)
{
  if (l == nullptr) return;

  stateChanged += Delegate(l, &IStateListener::HandleStateChange);
}

void
IStateListener::Events
::RemoveListener(IStateListener* l)
{
  if (l == nullptr) return;

  stateChanged -= Delegate(l, &IStateListener::HandleStateChange);
}

IStateListener::~IStateListener()
{
}

}
