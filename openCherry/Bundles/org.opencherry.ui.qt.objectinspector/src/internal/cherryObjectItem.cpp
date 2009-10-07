/*=========================================================================
 
 Program:   openCherry Platform
 Language:  C++
 Date:      $Date$
 Version:   $Revision$
 
 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.
 
 =========================================================================*/

#include "cherryObjectItem.h"

namespace cherry
{

ObjectItem::ObjectItem()
{
  type = CLASS;
  className = 0;
  parent = 0;
}

ObjectItem::ObjectItem(const char* className) :
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
  QListIterator<ObjectItem*> iter(children);
  while (iter.hasNext())
  {
    delete iter.next();
  }
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
    std::string str(className);
    return str == item.className;
  }
  case INSTANCE:
    return obj == item.obj;
  case SMARTPOINTER:
    return spId == item.spId;
  };
}

}
