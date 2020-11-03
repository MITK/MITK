/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYOBJECTITEM_H_
#define BERRYOBJECTITEM_H_

#include <QList>

#include <berryObject.h>

namespace berry
{

struct ObjectItem
{

  enum Type
  {
    CLASS = 0, INSTANCE = 1, SMARTPOINTER = 2
  };

  union
  {
    const Object* obj;
    unsigned int spId;
  };

  QString className;

  Type type;

  QList<ObjectItem*> children;
  ObjectItem* parent;

  ObjectItem();

  ObjectItem(const QString& className);

  ObjectItem(const Object* obj, ObjectItem* parent);

  ObjectItem(unsigned int spId, ObjectItem* parent);

  ObjectItem(const ObjectItem& item);

  ~ObjectItem();

  ObjectItem& operator=(const ObjectItem& item);

  bool operator==(const ObjectItem& item) const;

};

}

#endif /* BERRYOBJECTITEM_H_ */
