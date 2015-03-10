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

#include "berryIShellListener.h"
#include "berryShell.h"

namespace berry {

void
IShellListener::Events
::AddListener(IShellListener* listener)
{
  if (listener == NULL) return;

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
  if (listener == NULL) return;

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
