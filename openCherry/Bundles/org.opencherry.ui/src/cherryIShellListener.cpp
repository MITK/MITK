/*=========================================================================

 Program:   openCherry Platform
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

#include "cherryIShellListener.h"
#include "cherryShell.h"

namespace cherry {

void
IShellListener::Events
::AddListener(IShellListener::Pointer listener)
{
  if (listener.IsNull()) return;

  shellActivated += Delegate(listener.GetPointer(), &IShellListener::ShellActivated);
  shellClosed += Delegate(listener.GetPointer(), &IShellListener::ShellClosed);
  shellDeactivated += Delegate(listener.GetPointer(), &IShellListener::ShellDeactivated);
  shellDeiconified += Delegate(listener.GetPointer(), &IShellListener::ShellDeiconified);
  shellIconified += Delegate(listener.GetPointer(), &IShellListener::ShellIconified);
}

void
IShellListener::Events
::RemoveListener(IShellListener::Pointer listener)
{
  if (listener.IsNull()) return;

  shellActivated -= Delegate(listener.GetPointer(), &IShellListener::ShellActivated);
  shellClosed -= Delegate(listener.GetPointer(), &IShellListener::ShellClosed);
  shellDeactivated -= Delegate(listener.GetPointer(), &IShellListener::ShellDeactivated);
  shellDeiconified -= Delegate(listener.GetPointer(), &IShellListener::ShellDeiconified);
  shellIconified -= Delegate(listener.GetPointer(), &IShellListener::ShellIconified);
}

void IShellListener::ShellActivated(ShellEvent::Pointer e) 
{}
void IShellListener::ShellClosed(ShellEvent::Pointer e) 
{}
void IShellListener::ShellDeactivated(ShellEvent::Pointer e) 
{}
void IShellListener::ShellDeiconified(ShellEvent::Pointer e) 
{}
void IShellListener::ShellIconified(ShellEvent::Pointer e) 
{}

}
