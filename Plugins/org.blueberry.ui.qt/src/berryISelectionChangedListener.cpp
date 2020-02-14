/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryISelectionChangedListener.h"
#include "berryISelectionProvider.h"

namespace berry {

void
ISelectionChangedListener::Events
::AddListener(ISelectionChangedListener* listener)
{
  if (listener == nullptr) return;

  this->selectionChanged += Delegate(listener, &ISelectionChangedListener::SelectionChanged);
}

void
ISelectionChangedListener::Events
::RemoveListener(ISelectionChangedListener* listener)
{
  if (listener == nullptr) return;

  this->selectionChanged -= Delegate(listener, &ISelectionChangedListener::SelectionChanged);
}

ISelectionChangedListener::~ISelectionChangedListener()
{
}

}
