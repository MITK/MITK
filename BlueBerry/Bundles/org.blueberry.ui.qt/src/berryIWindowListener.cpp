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

#include "berryIWindowListener.h"

namespace berry {

void
IWindowListener::Events
::AddListener(IWindowListener::Pointer listener)
{
  if (listener.IsNull()) return;

  windowActivated += Delegate(listener.GetPointer(), &IWindowListener::WindowActivated);
  windowDeactivated += Delegate(listener.GetPointer(), &IWindowListener::WindowDeactivated);
  windowClosed += Delegate(listener.GetPointer(), &IWindowListener::WindowClosed);
  windowOpened += Delegate(listener.GetPointer(), &IWindowListener::WindowOpened);
}

void
IWindowListener::Events
::RemoveListener(IWindowListener::Pointer listener)
{
  if (listener.IsNull()) return;

  windowActivated -= Delegate(listener.GetPointer(), &IWindowListener::WindowActivated);
  windowDeactivated -= Delegate(listener.GetPointer(), &IWindowListener::WindowDeactivated);
  windowClosed -= Delegate(listener.GetPointer(), &IWindowListener::WindowClosed);
  windowOpened -= Delegate(listener.GetPointer(), &IWindowListener::WindowOpened);
}

}
