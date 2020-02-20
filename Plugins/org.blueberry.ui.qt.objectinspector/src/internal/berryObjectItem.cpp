/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryObjectItem.h"

namespace berry
{

ObjectItem::ObjectItem()
{
  type = CLASS;
  parent = nullptr;
}

ObjectItem::ObjectItem(const QString& className) :
  className(className), type(CLASS), parent(nullptr)
{
}

ObjectItem::ObjectItem(const Object* obj, ObjectItem* parent) :
  obj(obj), type(INSTANCE), parent(parent)
{
}

ObjectItem::ObjectItem(unsigned int spId, ObjectItem* parent) :
  spId(spId), type(SMARTPOINTER), parent(parent)
{
}

ObjectItem::ObjectItem(const ObjectItem& item) :
  type(item.type), children(item.children), parent(item.parent)
{
  switch (type)
  {
  case CLASS:
    className = item.className;
    break;
  case INSTANCE:
    obj = item.obj;
    break;
  case SMARTPOINTER:
    spId = item.spId;
    break;
  }
}

ObjectItem::~ObjectItem()
{
  qDeleteAll(children);
}

ObjectItem& ObjectItem::operator=(const ObjectItem& item)
{
  type = item.type;
  children = item.children;
  parent = item.parent;

  switch (type)
  {
  case CLASS:
    className = item.className;
    break;
  case INSTANCE:
    obj = item.obj;
    break;
  case SMARTPOINTER:
    spId = item.spId;
    break;
  }

  return *this;
}

bool ObjectItem::operator==(const ObjectItem& item) const
{
  if (type != item.type)
    return false;
  switch (type)
  {
  case CLASS:
  {
    return className == item.className;
  }
  case INSTANCE:
    return obj == item.obj;
  case SMARTPOINTER:
    return spId == item.spId;
  };
  return false;
}

}
