/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryICommandManagerListener.h"

#include "berryCommandManagerEvent.h"

namespace berry
{

void ICommandManagerListener::Events::AddListener(ICommandManagerListener* l)
{
  if (l == nullptr) return;

  commandManagerChanged += Delegate(l, &ICommandManagerListener::CommandManagerChanged);
}

void ICommandManagerListener::Events::RemoveListener(ICommandManagerListener* l)
{
  if (l == nullptr) return;

  commandManagerChanged -= Delegate(l, &ICommandManagerListener::CommandManagerChanged);
}

ICommandManagerListener::~ICommandManagerListener()
{
}

}

