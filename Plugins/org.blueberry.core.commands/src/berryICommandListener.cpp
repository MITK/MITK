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
  if (l == 0) return;

  commandChanged += Delegate(l, &ICommandListener::CommandChanged);
}

void
ICommandListener::Events
::RemoveListener(ICommandListener* l)
{
  if (l == 0) return;

  commandChanged -= Delegate(l, &ICommandListener::CommandChanged);
}

ICommandListener::~ICommandListener()
{
}

}



