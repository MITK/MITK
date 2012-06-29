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

#include "berryExtensionHandle.h"
#include "berryExtensionPointHandle.h"
#include "berryIObjectManager.h"
#include "berryRegistryDelta.h"

namespace berry {

class ExtensionDeltaData : public QSharedData
{
public:

  ExtensionDeltaData()
    : kind(ExtensionDelta::ADDED), extension(0), extensionPoint(0), containingDelta(NULL)
  {
  }

  QString GetKindString() const
  {
    switch (kind)
    {
    case ExtensionDelta::ADDED :
      return "ADDED";
    case ExtensionDelta::REMOVED :
      return "REMOVED";
    }
    return "UNKNOWN";
  }

  ExtensionDelta::Kind kind;
  int extension;
  int extensionPoint;
  RegistryDelta* containingDelta;
};

ExtensionDelta::ExtensionDelta()
  : d(new ExtensionDeltaData)
{
}

ExtensionDelta::ExtensionDelta(const ExtensionDelta &other)
  : d(other.d)
{
}

ExtensionDelta::~ExtensionDelta()
{
}

ExtensionDelta &ExtensionDelta::operator =(const ExtensionDelta &other)
{
  d = other.d;
  return *this;
}

bool ExtensionDelta::operator ==(const ExtensionDelta &other) const
{
  return d == other.d;
}

uint ExtensionDelta::GetHashCode() const
{
  return qHash(d.constData());
}

void ExtensionDelta::SetContainingDelta(RegistryDelta* containingDelta)
{
  d->containingDelta = containingDelta;
}

int ExtensionDelta::GetExtensionId() const
{
  return d->extension;
}

int ExtensionDelta::GetExtensionPointId() const
{
  return d->extensionPoint;
}

SmartPointer<IExtensionPoint> ExtensionDelta::GetExtensionPoint() const
{
  IExtensionPoint::Pointer xp(new ExtensionPointHandle(d->containingDelta->GetObjectManager(),
                                                       d->extensionPoint));
  return xp;
}

void ExtensionDelta::SetExtensionPoint(int extensionPoint)
{
  d->extensionPoint = extensionPoint;
}

ExtensionDelta::Kind ExtensionDelta::GetKind() const
{
  return d->kind;
}

SmartPointer<IExtension> ExtensionDelta::GetExtension() const
{
  IExtension::Pointer ext(new ExtensionHandle(d->containingDelta->GetObjectManager(),
                                              d->extension));
  return ext;
}

void ExtensionDelta::SetExtension(int extension)
{
  d->extension = extension;
}

void ExtensionDelta::SetKind(Kind kind) const
{
  d->kind = kind;
}

QString ExtensionDelta::ToString() const
{
  return "\n\t\t" + GetExtensionPoint()->GetUniqueIdentifier() + " - " +
      GetExtension()->GetNamespaceIdentifier() + '.' + GetExtension()->GetSimpleIdentifier() +
      " (" + d->GetKindString() + ")";
}

}

uint qHash(const berry::ExtensionDelta& delta)
{
  return delta.GetHashCode();
}
