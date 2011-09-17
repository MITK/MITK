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
