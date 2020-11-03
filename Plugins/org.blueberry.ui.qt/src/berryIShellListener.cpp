/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryIShellListener.h"
#include "berryShell.h"

namespace berry {

void
IShellListener::Events
::AddListener(IShellListener* listener)
{
  if (listener == nullptr) return;

  shellActivated += Delegate(listener, &IShellListener::ShellActivated);
  shellClosed += Delegate(listener, &IShellListener::ShellClosed);
  shellDeactivated += Delegate(listener, &IShellListener::ShellDeactivated);
  shellDeiconified += Delegate(listener, &IShellListener::ShellDeiconified);
  shellIconified += Delegate(listener, &IShellListener::ShellIconified);
}

void
IShellListener::Events
::RemoveListener(IShellListener* listener)
{
  if (listener == nullptr) return;

  shellActivated -= Delegate(listener, &IShellListener::ShellActivated);
  shellClosed -= Delegate(listener, &IShellListener::ShellClosed);
  shellDeactivated -= Delegate(listener, &IShellListener::ShellDeactivated);
  shellDeiconified -= Delegate(listener, &IShellListener::ShellDeiconified);
  shellIconified -= Delegate(listener, &IShellListener::ShellIconified);
}

IShellListener::~IShellListener()
{
}

void IShellListener::ShellActivated(const ShellEvent::Pointer& /*e*/)
{}
void IShellListener::ShellClosed(const ShellEvent::Pointer& /*e*/)
{}
void IShellListener::ShellDeactivated(const ShellEvent::Pointer& /*e*/)
{}
void IShellListener::ShellDeiconified(const ShellEvent::Pointer& /*e*/)
{}
void IShellListener::ShellIconified(const ShellEvent::Pointer& /*e*/)
{}

}
