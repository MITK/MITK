/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryGuiTkISelectionListener.h"

namespace berry {

namespace GuiTk {

ISelectionListener::~ISelectionListener()
{
}

void
ISelectionListener::Events
::AddListener(ISelectionListener::Pointer l)
{
  if (l.IsNull()) return;

  selected += Delegate(l.GetPointer(), &ISelectionListener::WidgetSelected);
  defaultSelected += Delegate(l.GetPointer(), &ISelectionListener::WidgetDefaultSelected);
}

void
ISelectionListener::Events
::RemoveListener(ISelectionListener::Pointer l)
{
  if (l.IsNull()) return;

  selected -= Delegate(l.GetPointer(), &ISelectionListener::WidgetSelected);
  defaultSelected -= Delegate(l.GetPointer(), &ISelectionListener::WidgetDefaultSelected);
}

}

}
