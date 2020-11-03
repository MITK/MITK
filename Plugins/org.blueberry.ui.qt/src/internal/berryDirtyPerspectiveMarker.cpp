/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryDirtyPerspectiveMarker.h"

namespace berry {

DirtyPerspectiveMarker::DirtyPerspectiveMarker(const QString& id)
  : perspectiveId(id)
{
}

uint DirtyPerspectiveMarker::HashCode() const
{
  return qHash(perspectiveId);
}

bool DirtyPerspectiveMarker::operator==(const Object* o) const
{
  if (const DirtyPerspectiveMarker* other = dynamic_cast<const DirtyPerspectiveMarker*>(o))
  {
    return perspectiveId == other->perspectiveId;
  }
  return false;
}

}
