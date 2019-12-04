/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryExtension.h"

#include "berryExtensionRegistry.h"
#include "berryRegistryContributor.h"
#include "berryRegistryObjectManager.h"

namespace berry
{

const int Extension::LABEL = 0; //The human readable name of the extension
const int Extension::XPT_NAME = 1; // The fully qualified name of the extension point to which this extension is attached to
const int Extension::CONTRIBUTOR_ID = 2; // ID of the actual contributor of this extension
const int Extension::EXTRA_SIZE = 3;

QList<QString> Extension::GetExtraData() const
{
  //The extension has been created by parsing, or does not have any extra data
  if (NoExtraData())
  {
    return extraInformation;
  }

//  //The extension has been loaded from the cache.
//  if (extraInformation.empty())
//  {
//    extraInformation = registry->GetTableReader()->LoadExtensionExtraData(GetExtraDataOffset());
//  }
  return extraInformation;
}

Extension::Extension(ExtensionRegistry* registry, bool persist)
  : RegistryObject(registry, persist)
{
  for (int i = 0; i < EXTRA_SIZE; ++i)
    extraInformation << QString();
}

Extension::Extension(int self, const QString& simpleId, const QString& namespaze,
          const QList<int>& children, int extraData,
          ExtensionRegistry* registry, bool persist)
  : RegistryObject(registry, persist), simpleId(simpleId), namespaceIdentifier(namespaze)
{
  for (int i = 0; i < EXTRA_SIZE; ++i)
    extraInformation << QString();

  SetObjectId(self);
  SetRawChildren(children);
  SetExtraDataOffset(extraData);
}

QString Extension::GetExtensionPointIdentifier() const
{
  return GetExtraData()[XPT_NAME];
}

QString Extension::GetSimpleIdentifier() const
{
  return simpleId;
}

QString Extension::GetUniqueIdentifier() const
{
  return simpleId.isEmpty() ? QString() : this->GetNamespaceIdentifier() + '.' + simpleId;
}

void Extension::SetExtensionPointIdentifier(const QString& value)
{
  extraInformation[XPT_NAME] = value;
}

void Extension::SetSimpleIdentifier(const QString& value)
{
  simpleId = value;
}

QString Extension::GetLabel() const
{
  return GetExtraData()[LABEL];
}

void Extension::SetLabel(const QString& value)
{
  extraInformation[LABEL] = value;
}

QString Extension::GetContributorId() const
{
  return GetExtraData()[CONTRIBUTOR_ID];
}

void Extension::SetContributorId(const QString& value)
{
  extraInformation[CONTRIBUTOR_ID] = value;
}

void Extension::SetNamespaceIdentifier(const QString& value)
{
  namespaceIdentifier = value;
}

QString Extension::GetLabelAsIs() const
{
  return GetExtraData()[LABEL];
}

QString Extension::GetLabel(const QLocale& /*locale*/) const
{
  registry->LogMultiLangError();
  return GetLabel();
}

SmartPointer<IContributor> Extension::GetContributor() const
{
  return registry->GetObjectManager()->GetContributor(GetContributorId());
}

QString Extension::GetNamespaceIdentifier() const
{
  return namespaceIdentifier;
}

QString Extension::ToString() const
{
  return GetUniqueIdentifier() + " -> " + GetExtensionPointIdentifier();
}

} // namespace berry
