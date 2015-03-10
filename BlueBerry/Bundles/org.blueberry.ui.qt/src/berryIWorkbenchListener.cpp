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

#include "berryIWorkbenchListener.h"

namespace berry {

void
IWorkbenchListener::Events
::AddListener(IWorkbenchListener* listener)
{
  if (listener == NULL) return;

  preShutdown += Delegate2(listener, &IWorkbenchListener::PreShutdown);
  postShutdown += Delegate1(listener, &IWorkbenchListener::PostShutdown);
}

void
IWorkbenchListener::Events
::RemoveListener(IWorkbenchListener* listener)
{
  if (listener == NULL) return;

  preShutdown -= Delegate2(listener, &IWorkbenchListener::PreShutdown);
  postShutdown -= Delegate1(listener, &IWorkbenchListener::PostShutdown);
}

IWorkbenchListener::~IWorkbenchListener()
{
}

}
