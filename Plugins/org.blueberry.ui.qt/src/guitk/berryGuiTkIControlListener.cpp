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
