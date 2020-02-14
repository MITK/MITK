/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryGuiTkIControlListener.h"

namespace berry {

namespace GuiTk {

IControlListener::~IControlListener()
{
}

void
IControlListener::Events
::AddListener(IControlListener::Pointer l)
{
  if (l.IsNull()) return;

  Types types = l->GetEventTypes();

  if (types & MOVED)
    movedEvent += Delegate(l.GetPointer(), &IControlListener::ControlMoved);
  if (types & RESIZED)
    resizedEvent += Delegate(l.GetPointer(), &IControlListener::ControlResized);
  if (types & ACTIVATED)
    activatedEvent += Delegate(l.GetPointer(), &IControlListener::ControlActivated);
  if (types & DESTROYED)
    destroyedEvent += Delegate(l.GetPointer(), &IControlListener::ControlDestroyed);
}

void
IControlListener::Events
::RemoveListener(IControlListener::Pointer l)
{
  if (l.IsNull()) return;

  movedEvent -= Delegate(l.GetPointer(), &IControlListener::ControlMoved);
  resizedEvent -= Delegate(l.GetPointer(), &IControlListener::ControlResized);
  activatedEvent -= Delegate(l.GetPointer(), &IControlListener::ControlActivated);
  destroyedEvent -= Delegate(l.GetPointer(), &IControlListener::ControlDestroyed);
}

}

}
