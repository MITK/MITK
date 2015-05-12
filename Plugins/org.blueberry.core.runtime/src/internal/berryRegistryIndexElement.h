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

  berryObjectMacro(berry::RegistryIndexElement)

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
