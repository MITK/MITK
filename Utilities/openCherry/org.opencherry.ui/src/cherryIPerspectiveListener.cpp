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

#include "cherryIPerspectiveListener.h"

#include "cherryIWorkbenchPage.h"

namespace cherry {

void
IPerspectiveListener::Events
::AddListener(IPerspectiveListener::Pointer listener)
{
  if (listener.IsNull()) return;

  this->perspectiveActivated += Delegate2(listener.GetPointer(), &IPerspectiveListener::PerspectiveActivated);
  this->perspectiveChanged += PerspChangedDelegate(listener.GetPointer(), &IPerspectiveListener::PerspectiveChanged);
  this->perspectivePartChanged += PerspPartChangedDelegate(listener.GetPointer(), &IPerspectiveListener::PerspectiveChanged);
  this->perspectiveOpened += Delegate2(listener.GetPointer(), &IPerspectiveListener::PerspectiveOpened);
  this->perspectiveClosed += Delegate2(listener.GetPointer(), &IPerspectiveListener::PerspectiveClosed);
  this->perspectiveDeactivated += Delegate2(listener.GetPointer(), &IPerspectiveListener::PerspectiveDeactivated);
  this->perspectiveSavedAs += PerspSavedAsDelegate(listener.GetPointer(), &IPerspectiveListener::PerspectiveSavedAs);
  this->perspectivePreDeactivate += Delegate2(listener.GetPointer(), &IPerspectiveListener::PerspectivePreDeactivate);
}

void
IPerspectiveListener::Events
::RemoveListener(IPerspectiveListener::Pointer listener)
{
  if (listener.IsNull()) return;

  this->perspectiveActivated -= Delegate2(listener.GetPointer(), &IPerspectiveListener::PerspectiveActivated);
  this->perspectiveChanged -= PerspChangedDelegate(listener.GetPointer(), &IPerspectiveListener::PerspectiveChanged);
  this->perspectivePartChanged -= PerspPartChangedDelegate(listener.GetPointer(), &IPerspectiveListener::PerspectiveChanged);
  this->perspectiveOpened -= Delegate2(listener.GetPointer(), &IPerspectiveListener::PerspectiveOpened);
  this->perspectiveClosed -= Delegate2(listener.GetPointer(), &IPerspectiveListener::PerspectiveClosed);
  this->perspectiveDeactivated -= Delegate2(listener.GetPointer(), &IPerspectiveListener::PerspectiveDeactivated);
  this->perspectiveSavedAs -= PerspSavedAsDelegate(listener.GetPointer(), &IPerspectiveListener::PerspectiveSavedAs);
  this->perspectivePreDeactivate -= Delegate2(listener.GetPointer(), &IPerspectiveListener::PerspectivePreDeactivate);
}

}
