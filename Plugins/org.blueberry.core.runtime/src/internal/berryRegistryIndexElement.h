/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYREGISTRYINDEXELEMENT_H
#define BERRYREGISTRYINDEXELEMENT_H

#include "berryKeyedElement.h"

#include "berryRegistryIndexChildren.h"

namespace berry {

class RegistryIndexChildren;

class RegistryIndexElement : public KeyedElement
{

private:

  // The key on which indexing is done
  const QString key;

  // Extension points matching the key
  RegistryIndexChildren extensionPoints;

  // Extensions matching the key
  RegistryIndexChildren extensions;

public:

  berryObjectMacro(berry::RegistryIndexElement);

  RegistryIndexElement(const QString& key);

  RegistryIndexElement(const QString& key, const QList<int>& extensionPoints,
                       const QList<int>& extensions);

  QList<int> GetExtensions() const;

  QList<int> GetExtensionPoints() const;

  bool UpdateExtension(int id, bool add);

  bool UpdateExtensions(const QList<int>& IDs, bool add);

  bool UpdateExtensionPoint(int id, bool add);

  bool UpdateExtensionPoints(const QList<int>& IDs, bool add);

  //Implements the KeyedElement interface
  QString GetKey() const override;

private:

  bool IsEqual(const KeyedElement& other) const override;
};

}

#endif // BERRYREGISTRYINDEXELEMENT_H
