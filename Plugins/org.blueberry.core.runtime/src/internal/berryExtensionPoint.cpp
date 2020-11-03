/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryExtensionPoint.h"

#include "berryExtensionRegistry.h"
#include "berryRegistryContributor.h"
#include "berryRegistryObjectManager.h"

#include <QDir>

namespace berry {

const int ExtensionPoint::LABEL = 0; //The human readable name for the extension point
const int ExtensionPoint::SCHEMA = 1; //The schema of the extension point
const int ExtensionPoint::QUALIFIED_NAME = 2; //The fully qualified name of the extension point
const int ExtensionPoint::NAMESPACE = 3; //The name of the namespace of the extension point
const int ExtensionPoint::CONTRIBUTOR_ID = 4; //The ID of the actual contributor of the extension point
const int ExtensionPoint::EXTRA_SIZE = 5;

QList<QString> ExtensionPoint::GetExtraData() const
{
  //The extension point has been created by parsing, or does not have any extra data
  if (NoExtraData())
  { //When this is true, the extraInformation is always a String[]. This happens when the object is created by the parser.
    return extraInformation;
  }

//  //The extension point has been loaded from the cache.
//  if (extraInformation.empty())
//  {
//    extraInformation = registry->GetTableReader()->LoadExtensionPointExtraData(GetExtraDataOffset());
//  }
  return extraInformation;
}

ExtensionPoint::ExtensionPoint(ExtensionRegistry* registry, bool persist)
  : RegistryObject(registry, persist)
{
  for (int i = 0; i < EXTRA_SIZE; ++i)
    extraInformation << QString();
}

ExtensionPoint::ExtensionPoint(int self, const QList<int>& children, int dataOffset,
               ExtensionRegistry* registry, bool persist)
  : RegistryObject(registry, persist)
{
  for (int i = 0; i < EXTRA_SIZE; ++i)
    extraInformation << QString();

  SetObjectId(self);
  SetRawChildren(children);
  SetExtraDataOffset(dataOffset);
}

QString ExtensionPoint::GetSimpleIdentifier() const
{
  return GetUniqueIdentifier().mid(GetUniqueIdentifier().lastIndexOf('.') + 1);
}

QString ExtensionPoint::GetSchemaReference() const
{
  return QDir::fromNativeSeparators(GetExtraData()[SCHEMA]);
}

QString ExtensionPoint::GetLabel() const
{
  return GetExtraData()[LABEL];
}

QString ExtensionPoint::GetUniqueIdentifier() const
{
  return GetExtraData()[QUALIFIED_NAME];
}

QString ExtensionPoint::GetNamespace() const
{
  return GetExtraData()[NAMESPACE];
}

QString ExtensionPoint::GetContributorId() const
{
  return GetExtraData()[CONTRIBUTOR_ID];
}

void ExtensionPoint::SetSchema(const QString& value)
{
  extraInformation[SCHEMA] = value;
}

void ExtensionPoint::SetLabel(const QString& value)
{
  extraInformation[LABEL] = value;
}

void ExtensionPoint::SetUniqueIdentifier(const QString& value)
{
  extraInformation[QUALIFIED_NAME] = value;
}

void ExtensionPoint::SetNamespace(const QString& value)
{
  extraInformation[NAMESPACE] = value;
}

void ExtensionPoint::SetContributorId(const QString id)
{
  extraInformation[CONTRIBUTOR_ID] = id;
}

QString ExtensionPoint::GetLabelAsIs() const
{
  return GetExtraData()[LABEL];
}

QString ExtensionPoint::GetLabel(const QLocale& /*locale*/)
{
  registry->LogMultiLangError();
  return GetLabel();
}

SmartPointer<IContributor> ExtensionPoint::GetContributor() const
{
  return registry->GetObjectManager()->GetContributor(GetContributorId());
}

QString ExtensionPoint::ToString() const
{
  return GetUniqueIdentifier();
}

}
