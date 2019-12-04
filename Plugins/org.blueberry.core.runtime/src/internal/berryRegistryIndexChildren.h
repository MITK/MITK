/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYREGISTRYINDEXCHILDREN_H
#define BERRYREGISTRYINDEXCHILDREN_H

#include <QList>

namespace berry {

class RegistryIndexChildren
{

private:

  QList<int> children;

public:

  RegistryIndexChildren();

  RegistryIndexChildren(const QList<int>& children);

  QList<int> GetChildren() const;

  int FindChild(int id) const;

  bool UnlinkChild(int id);

  bool LinkChild(int id);

  bool LinkChildren(const QList<int>& IDs);

  bool UnlinkChildren(const QList<int>& IDs);

};

}

#endif // BERRYREGISTRYINDEXCHILDREN_H
