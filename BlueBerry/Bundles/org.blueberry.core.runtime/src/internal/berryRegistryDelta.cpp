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

#include "berryExtensionDelta.h"
#include "berryRegistryDelta.h"
#include "berryIExtension.h"

#include <QSet>

namespace berry {

class RegistryDeltaData : public QSharedData
{
public:

  RegistryDeltaData()
    : objectManager(NULL)
  {
  }

  QSet<ExtensionDelta> extensionDeltas; //the extension deltas (each element indicate the type of the delta)
  SmartPointer<IObjectManager> objectManager; //The object manager from which all the objects contained in the deltas will be found.

};

RegistryDelta::RegistryDelta()
  : d(new RegistryDeltaData)
{
  //Nothing to do
}

RegistryDelta::~RegistryDelta()
{
}

int RegistryDelta::GetExtensionDeltasCount() const
{
  return d->extensionDeltas.size();
}

QList<ExtensionDelta> RegistryDelta::GetExtensionDeltas() const
{
  return d->extensionDeltas.toList();
}

QList<ExtensionDelta> RegistryDelta::GetExtensionDeltas(const QString& extensionPoint) const
{
  QList<ExtensionDelta> selectedExtDeltas;
  foreach (ExtensionDelta extensionDelta, d->extensionDeltas)
  {
    if (extensionDelta.GetExtension()->GetExtensionPointUniqueIdentifier() == extensionPoint)
    {
      selectedExtDeltas.push_back(extensionDelta);
    }
  }
  return selectedExtDeltas;
}

ExtensionDelta RegistryDelta::GetExtensionDelta(const QString& extensionPointId, const QString& extensionId) const
{
  foreach (ExtensionDelta extensionDelta, d->extensionDeltas)
  {
    IExtension::Pointer extension = extensionDelta.GetExtension();
    if (extension->GetExtensionPointUniqueIdentifier() == extensionPointId &&
        extension->GetUniqueIdentifier() == extensionId)
    {
      return extensionDelta;
    }
  }
  return ExtensionDelta();
}

void RegistryDelta::AddExtensionDelta(ExtensionDelta& extensionDelta)
{
  d->extensionDeltas.insert(extensionDelta);
  extensionDelta.SetContainingDelta(this);
}

QString RegistryDelta::ToString() const
{
  QString str("\n\tHost ");
  str += ": (";
  int i = 0;
  foreach (ExtensionDelta extensionDelta, d->extensionDeltas)
  {
    str += extensionDelta.ToString();
    if (i < d->extensionDeltas.size() -1) str += ", ";
  }
  str += ")";
  return str;
}

void RegistryDelta::SetObjectManager(const SmartPointer<IObjectManager>& objectManager)
{
  d->objectManager = objectManager;
  //TODO May want to add into the existing one here... if it is possible to have batching
}

SmartPointer<IObjectManager> RegistryDelta::GetObjectManager() const
{
  return d->objectManager;
}

}
