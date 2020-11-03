/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryICommandListener.h"

#include "berryCommand.h"
#include "berryCommandEvent.h"
#include "berryCommandCategory.h"
#include "berryState.h"
#include "berryIHandler.h"

namespace berry {

void
ICommandListener::Events
::AddListener(ICommandListener* l)
{
  if (l == nullptr) return;

  commandChanged += Delegate(l, &ICommandListener::CommandChanged);
}

void
ICommandListener::Events
::RemoveListener(ICommandListener* l)
{
  if (l == nullptr) return;

  commandChanged -= Delegate(l, &ICommandListener::CommandChanged);
}

ICommandListener::~ICommandListener()
{
}

}



