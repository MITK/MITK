/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYOBJECTSTRINGMAP_H
#define BERRYOBJECTSTRINGMAP_H

#include <berryObject.h>

#include <QHash>

namespace berry {

class ObjectStringMap : public QHash<QString,QString>, public Object
{
public:

  berryObjectMacro(berry::ObjectStringMap);

  ObjectStringMap();
  ObjectStringMap(const QHash<QString,QString>& other);

};

}

#endif // BERRYOBJECTSTRINGMAP_H
