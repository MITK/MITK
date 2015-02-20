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

#include "berryObjectItem.h"

namespace berry
{

ObjectItem::ObjectItem()
{
  type = CLASS;
  parent = 0;
}

ObjectItem::ObjectItem(const QString& className) :
  className(className), type(CLASS), parent(0)
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
