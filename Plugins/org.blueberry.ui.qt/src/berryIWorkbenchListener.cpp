/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryIWorkbenchListener.h"

namespace berry {

void
IWorkbenchListener::Events
::AddListener(IWorkbenchListener* listener)
{
  if (listener == nullptr) return;

  preShutdown += Delegate2(listener, &IWorkbenchListener::PreShutdown);
  postShutdown += Delegate1(listener, &IWorkbenchListener::PostShutdown);
}

void
IWorkbenchListener::Events
::RemoveListener(IWorkbenchListener* listener)
{
  if (listener == nullptr) return;

  preShutdown -= Delegate2(listener, &IWorkbenchListener::PreShutdown);
  postShutdown -= Delegate1(listener, &IWorkbenchListener::PostShutdown);
}

IWorkbenchListener::~IWorkbenchListener()
{
}

}
