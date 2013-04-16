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
#include "berryIHandler.h"

namespace berry {

void
ICommandListener::Events
::AddListener(ICommandListener::Pointer l)
{
  if (l.IsNull()) return;

  commandChanged += Delegate(l.GetPointer(), &ICommandListener::CommandChanged);
}

void
ICommandListener::Events
::RemoveListener(ICommandListener::Pointer l)
{
  if (l.IsNull()) return;

  commandChanged -= Delegate(l.GetPointer(), &ICommandListener::CommandChanged);
}

}



