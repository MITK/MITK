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
