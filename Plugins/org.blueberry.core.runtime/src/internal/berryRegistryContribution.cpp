/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryRegistryContribution.h"

#include "berryExtensionRegistry.h"
#include "berryInvalidRegistryObjectException.h"
#include "berryRegistryObjectManager.h"
#include "berryRegistryContributor.h"

namespace berry {

const int RegistryContribution::EXTENSION_POINT = 0;
const int RegistryContribution::EXTENSION = 1;

RegistryContribution::RegistryContribution(const QString& contributorId, ExtensionRegistry* registry, bool persist)
  : registry(registry), contributorId(contributorId), persist(persist)
{
}

void RegistryContribution::MergeContribution(const RegistryContribution::Pointer& addContribution)
{
  Q_ASSERT(contributorId == addContribution->contributorId);
  Q_ASSERT(registry == addContribution->registry);

  // persist?
  // Old New Result
  //  F   F   F
  //  F   T   T  => needs to be adjusted
  //  T   F   T
  //  T   T   T
  if (ShouldPersist() != addContribution->ShouldPersist())
    persist = true;

  QList<int> existing = GetRawChildren();
  QList<int> addition = addContribution->GetRawChildren();

  int extensionPoints = existing[EXTENSION_POINT] + addition[EXTENSION_POINT];
  int extensions = existing[EXTENSION] + addition[EXTENSION];
  children.clear();
  children.push_back(extensionPoints);
  children.push_back(extensions);
  children.append(existing.mid(2, existing[EXTENSION_POINT]));
  children.append(addition.mid(2, addition[EXTENSION_POINT]));
  children.append(existing.mid(2 + existing[EXTENSION_POINT]));
  children.append(addition.mid(2 + addition[EXTENSION_POINT]));
}

void RegistryContribution::SetRawChildren(const QList<int>& children)
{
  this->children = children;
}

QString RegistryContribution::GetContributorId() const
{
  return contributorId;
}

QList<int> RegistryContribution::GetRawChildren() const
{
  return children;
}

QList<int> RegistryContribution::GetExtensions() const
{
  return children.mid(2 + children[EXTENSION_POINT]);
}

QList<int> RegistryContribution::GetExtensionPoints() const
{
  return children.mid(2, children[EXTENSION_POINT]);
}

bool RegistryContribution::IsEqual(const KeyedElement& other) const
{
  return contributorId == static_cast<const RegistryContribution&>(other).contributorId;
}

QString RegistryContribution::GetDefaultNamespace() const
{
  if (defaultNamespace.isNull())
  {
    defaultNamespace = registry->GetObjectManager()->GetContributor(contributorId)->GetName();
  }
  return defaultNamespace;
}

QString RegistryContribution::ToString() const
{
  return QString("Contribution: ") + contributorId + " in namespace" + GetDefaultNamespace();
}

QString RegistryContribution::GetKey() const
{
  return contributorId;
}

bool RegistryContribution::ShouldPersist() const
{
  return persist;
}

void RegistryContribution::UnlinkChild(int id)
{
  // find index of the child being unlinked:
  int index = -1;
  for (int i = 2; i < children.size(); i++)
  {
    if (children[i] == id)
    {
      index = i;
      break;
    }
  }
  if (index == -1)
    throw InvalidRegistryObjectException();

  // copy all array except one element at index
  children.removeAt(index);

  // fix sizes
  if (index < children[EXTENSION_POINT] + 2)
    children[EXTENSION_POINT]--;
  else
    children[EXTENSION]--;
}

bool RegistryContribution::IsEmpty() const
{
  return (children[EXTENSION_POINT] == 0 || children[EXTENSION] == 0);
}

bool RegistryContribution::HasChild(int id) const
{
  for (int i = 2; i < children.size(); i++)
  {
    if (children[i] == id)
      return true;
  }
  return false;
}

}
