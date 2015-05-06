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

#include "berryISelectionChangedListener.h"
#include "berryISelectionProvider.h"

namespace berry {

void
ISelectionChangedListener::Events
::AddListener(ISelectionChangedListener* listener)
{
  if (listener == NULL) return;

  this->selectionChanged += Delegate(listener, &ISelectionChangedListener::SelectionChanged);
}

void
ISelectionChangedListener::Events
::RemoveListener(ISelectionChangedListener* listener)
{
  if (listener == NULL) return;

  this->selectionChanged -= Delegate(listener, &ISelectionChangedListener::SelectionChanged);
}

ISelectionChangedListener::~ISelectionChangedListener()
{
}

}
