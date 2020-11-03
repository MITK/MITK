/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryIWindowListener.h"

namespace berry {

void
IWindowListener::Events
::AddListener(IWindowListener* listener)
{
  if (listener == nullptr) return;

  windowActivated += Delegate(listener, &IWindowListener::WindowActivated);
  windowDeactivated += Delegate(listener, &IWindowListener::WindowDeactivated);
  windowClosed += Delegate(listener, &IWindowListener::WindowClosed);
  windowOpened += Delegate(listener, &IWindowListener::WindowOpened);
}

void
IWindowListener::Events
::RemoveListener(IWindowListener* listener)
{
  if (listener == nullptr) return;

  windowActivated -= Delegate(listener, &IWindowListener::WindowActivated);
  windowDeactivated -= Delegate(listener, &IWindowListener::WindowDeactivated);
  windowClosed -= Delegate(listener, &IWindowListener::WindowClosed);
  windowOpened -= Delegate(listener, &IWindowListener::WindowOpened);
}

IWindowListener::~IWindowListener()
{
}

}
