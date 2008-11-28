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

#include "cherryIWorkbenchListener.h"

namespace cherry {

void
IWorkbenchListener::Events
::AddListener(IWorkbenchListener::Pointer listener)
{
  if (listener.IsNull()) return;

  preShutdown += Delegate2(listener.GetPointer(), &IWorkbenchListener::PreShutdown);
  postShutdown += Delegate1(listener.GetPointer(), &IWorkbenchListener::PostShutdown);
}

void
IWorkbenchListener::Events
::RemoveListener(IWorkbenchListener::Pointer listener)
{
  if (listener.IsNull()) return;

  preShutdown -= Delegate2(listener.GetPointer(), &IWorkbenchListener::PreShutdown);
  postShutdown -= Delegate1(listener.GetPointer(), &IWorkbenchListener::PostShutdown);
}

}
