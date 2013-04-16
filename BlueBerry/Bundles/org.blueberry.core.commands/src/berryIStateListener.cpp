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

#include "berryIStateListener.h"

#include "berryState.h"

namespace berry {

void
IStateListener::Events
::AddListener(IStateListener::Pointer l)
{
  if (l.IsNull()) return;

  stateChanged += Delegate(l.GetPointer(), &IStateListener::HandleStateChange);
}

void
IStateListener::Events
::RemoveListener(IStateListener::Pointer l)
{
  if (l.IsNull()) return;

  stateChanged -= Delegate(l.GetPointer(), &IStateListener::HandleStateChange);
}

}


