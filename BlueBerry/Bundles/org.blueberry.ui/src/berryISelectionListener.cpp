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

#include "berryISelectionListener.h"

namespace berry {

void
ISelectionListener::Events
::AddListener(ISelectionListener::Pointer listener)
{
  if (listener.IsNull()) return;

  this->selectionChanged += Delegate(listener.GetPointer(), &ISelectionListener::SelectionChanged);
}

void
ISelectionListener::Events
::RemoveListener(ISelectionListener::Pointer listener)
{
  if (listener.IsNull()) return;

  this->selectionChanged -= Delegate(listener.GetPointer(), &ISelectionListener::SelectionChanged);
}

}
