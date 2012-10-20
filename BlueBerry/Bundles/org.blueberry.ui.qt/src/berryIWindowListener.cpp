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
::AddListener(IWindowListener* listener)
{
  if (listener == NULL) return;

  windowActivated += Delegate(listener, &IWindowListener::WindowActivated);
  windowDeactivated += Delegate(listener, &IWindowListener::WindowDeactivated);
  windowClosed += Delegate(listener, &IWindowListener::WindowClosed);
  windowOpened += Delegate(listener, &IWindowListener::WindowOpened);
}

void
IWindowListener::Events
::RemoveListener(IWindowListener* listener)
{
  if (listener == NULL) return;

  windowActivated -= Delegate(listener, &IWindowListener::WindowActivated);
  windowDeactivated -= Delegate(listener, &IWindowListener::WindowDeactivated);
  windowClosed -= Delegate(listener, &IWindowListener::WindowClosed);
  windowOpened -= Delegate(listener, &IWindowListener::WindowOpened);
}

IWindowListener::~IWindowListener()
{
}

}
