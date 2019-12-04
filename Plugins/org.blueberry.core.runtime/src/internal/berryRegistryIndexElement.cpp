/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryRegistryIndexElement.h"

#include "berryRegistryIndexChildren.h"

namespace berry {

RegistryIndexElement::RegistryIndexElement(const QString& key)
  : key(key)
{
}

RegistryIndexElement::RegistryIndexElement(const QString& key, const QList<int>& extensionPoints,
                     const QList<int>& extensions)
  : key(key), extensionPoints(extensionPoints), extensions(extensions)
{
}

QList<int> RegistryIndexElement::GetExtensions() const
{
  return extensions.GetChildren();
}

QList<int> RegistryIndexElement::GetExtensionPoints() const
{
  return extensionPoints.GetChildren();
}

bool RegistryIndexElement::UpdateExtension(int id, bool add)
{
  if (add)
    return extensions.LinkChild(id);
  else
    return extensions.UnlinkChild(id);
}

bool RegistryIndexElement::UpdateExtensions(const QList<int>& IDs, bool add)
{
  if (add)
    return extensions.LinkChildren(IDs);
  else
    return extensions.UnlinkChildren(IDs);
}

bool RegistryIndexElement::UpdateExtensionPoint(int id, bool add)
{
  if (add)
    return extensionPoints.LinkChild(id);
  else
    return extensionPoints.UnlinkChild(id);
}

bool RegistryIndexElement::UpdateExtensionPoints(const QList<int>& IDs, bool add)
{
  if (add)
    return extensionPoints.LinkChildren(IDs);
  else
    return extensionPoints.UnlinkChildren(IDs);
}

//Implements the KeyedElement interface
QString RegistryIndexElement::GetKey() const
{
  return key;
}

bool RegistryIndexElement::IsEqual(const KeyedElement& other) const
{
  return key == static_cast<const RegistryIndexElement&>(other).key;
}

}
