/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryISelectionListener.h"

namespace berry {

void
ISelectionListener::Events
::AddListener(ISelectionListener* listener)
{
  if (listener == nullptr) return;

  this->selectionChanged += Delegate(listener, &ISelectionListener::SelectionChanged);
}

void
ISelectionListener::Events
::RemoveListener(ISelectionListener* listener)
{
  if (listener == nullptr) return;

  this->selectionChanged -= Delegate(listener, &ISelectionListener::SelectionChanged);
}

ISelectionListener::~ISelectionListener()
{
}

}
