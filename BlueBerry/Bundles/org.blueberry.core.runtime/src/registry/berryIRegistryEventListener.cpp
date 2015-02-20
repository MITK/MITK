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

#include "berryIRegistryEventListener.h"

namespace berry {

void IRegistryEventListener::Events::AddListener(IRegistryEventListener* l)
{
  if (l == NULL) return;

  extensionsAdded += ExtensionsAddedDelegate(l, &IRegistryEventListener::Added);
  extensionsRemoved += ExtensionsRemovedDelegate(l, &IRegistryEventListener::Removed);
  extensionPointsAdded += ExtensionPointsAddedDelegate(l, &IRegistryEventListener::Added);
  extensionPointsRemoved += ExtensionPointsRemovedDelegate(l, &IRegistryEventListener::Removed);
}

void IRegistryEventListener::Events::RemoveListener(IRegistryEventListener* l)
{
  if (l == NULL) return;

  extensionsAdded -= ExtensionsAddedDelegate(l, &IRegistryEventListener::Added);
  extensionsRemoved -= ExtensionsRemovedDelegate(l, &IRegistryEventListener::Removed);
  extensionPointsAdded -= ExtensionPointsAddedDelegate(l, &IRegistryEventListener::Added);
  extensionPointsRemoved -= ExtensionPointsRemovedDelegate(l, &IRegistryEventListener::Removed);
}

IRegistryEventListener::~IRegistryEventListener()
{
}

}
