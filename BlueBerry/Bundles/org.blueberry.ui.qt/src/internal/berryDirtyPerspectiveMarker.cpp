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
