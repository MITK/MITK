/*=========================================================================
 
 Program:   BlueBerry Platform
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
    const char* className;
    unsigned int spId;
  };

  Type type;

  QList<ObjectItem*> children;
  ObjectItem* parent;

  ObjectItem();

  ObjectItem(const char* className);

  ObjectItem(const Object* obj, ObjectItem* parent);

  ObjectItem(unsigned int spId, ObjectItem* parent);

  ObjectItem(const ObjectItem& item);

  ~ObjectItem();

  ObjectItem& operator=(const ObjectItem& item);

  bool operator==(const ObjectItem& item) const;

};

}

#endif /* BERRYOBJECTITEM_H_ */
