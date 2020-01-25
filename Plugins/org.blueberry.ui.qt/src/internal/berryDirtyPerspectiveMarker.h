/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYDIRTYPERSPECTIVEMARKER_H
#define BERRYDIRTYPERSPECTIVEMARKER_H

#include <berryObject.h>

#include <QString>

namespace berry {

class DirtyPerspectiveMarker : public Object
{
public:

  berryObjectMacro(DirtyPerspectiveMarker);

  QString perspectiveId;

  /**
   * @param id
   */
  DirtyPerspectiveMarker(const QString& id);

  uint HashCode() const override;

  bool operator==(const Object* o) const override;
};

}

#endif // BERRYDIRTYPERSPECTIVEMARKER_H
