/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryIRegistryEventListener.h"

namespace berry {

void IRegistryEventListener::Events::AddListener(IRegistryEventListener* l)
{
  if (l == nullptr) return;

  extensionsAdded += ExtensionsAddedDelegate(l, &IRegistryEventListener::Added);
  extensionsRemoved += ExtensionsRemovedDelegate(l, &IRegistryEventListener::Removed);
  extensionPointsAdded += ExtensionPointsAddedDelegate(l, &IRegistryEventListener::Added);
  extensionPointsRemoved += ExtensionPointsRemovedDelegate(l, &IRegistryEventListener::Removed);
}

void IRegistryEventListener::Events::RemoveListener(IRegistryEventListener* l)
{
  if (l == nullptr) return;

  extensionsAdded -= ExtensionsAddedDelegate(l, &IRegistryEventListener::Added);
  extensionsRemoved -= ExtensionsRemovedDelegate(l, &IRegistryEventListener::Removed);
  extensionPointsAdded -= ExtensionPointsAddedDelegate(l, &IRegistryEventListener::Added);
  extensionPointsRemoved -= ExtensionPointsRemovedDelegate(l, &IRegistryEventListener::Removed);
}

IRegistryEventListener::~IRegistryEventListener()
{
}

}
