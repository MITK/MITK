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

#ifndef BERRYOBJECTSTRINGMAP_H
#define BERRYOBJECTSTRINGMAP_H

#include <berryObject.h>

#include <QHash>

namespace berry {

class ObjectStringMap : public QHash<QString,QString>, public Object
{
public:

  berryObjectMacro(berry::ObjectStringMap)

  ObjectStringMap();
  ObjectStringMap(const QHash<QString,QString>& other);

};

}

#endif // BERRYOBJECTSTRINGMAP_H
