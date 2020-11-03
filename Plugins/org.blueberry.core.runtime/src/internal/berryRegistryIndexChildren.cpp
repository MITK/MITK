/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryRegistryIndexChildren.h"

namespace berry {

RegistryIndexChildren::RegistryIndexChildren()
{
}

RegistryIndexChildren::RegistryIndexChildren(const QList<int>& children)
  : children(children)
{
}

QList<int> RegistryIndexChildren::GetChildren() const
{
  return children;
}

int RegistryIndexChildren::FindChild(int id) const
{
  return children.indexOf(id);
}

bool RegistryIndexChildren::UnlinkChild(int id)
{
  return children.removeOne(id);
}

bool RegistryIndexChildren::LinkChild(int id)
{
  children.push_back(id);
  return true;
}

bool RegistryIndexChildren::LinkChildren(const QList<int>& IDs)
{
  children.append(IDs);
  return true;
}

bool RegistryIndexChildren::UnlinkChildren(const QList<int>& IDs)
{
  if (children.empty())
    return IDs.empty();

  for (int i = 0; i < IDs.size(); i++)
  {
    children.removeOne(IDs[i]);
  }
  return true;
}

}
