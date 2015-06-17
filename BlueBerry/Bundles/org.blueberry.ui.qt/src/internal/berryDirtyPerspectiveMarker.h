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

#ifndef BERRYDIRTYPERSPECTIVEMARKER_H
#define BERRYDIRTYPERSPECTIVEMARKER_H

#include <berryObject.h>

#include <QString>

namespace berry {

class DirtyPerspectiveMarker : public Object
{
public:

  berryObjectMacro(DirtyPerspectiveMarker)

  QString perspectiveId;

  /**
   * @param id
   */
  DirtyPerspectiveMarker(const QString& id);

  uint HashCode() const;

  bool operator==(const Object* o) const;
};

}

#endif // BERRYDIRTYPERSPECTIVEMARKER_H
