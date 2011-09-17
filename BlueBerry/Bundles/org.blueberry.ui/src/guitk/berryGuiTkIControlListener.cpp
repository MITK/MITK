/*=========================================================================

 Program:   BlueBerry Platform
 Language:  C++
 Date:      $Date$
 Version:   $Revision$

 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/

#include "berryGuiTkIControlListener.h"

namespace berry {

namespace GuiTk {

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
