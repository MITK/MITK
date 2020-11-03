/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYOSGIOBJECTLIST_H_
#define BERRYOSGIOBJECTLIST_H_

#include "berryMacros.h"

#include <berryObject.h>

#include <QList>

namespace berry {

template<typename T>
class ObjectList : public Object, public QList<T>
{
public:
  berryObjectMacro(berry::ObjectList<T>);

  ObjectList() : QList<T>() {}
  ObjectList(const QList<T>& l) : QList<T>(l) {}

  bool operator==(const Object* o) const override
  {
    if (const QList<T>* l = dynamic_cast<const QList<T>*>(o))
    {
      return QList<T>::operator ==(*l);
    }
    return false;
  }
};

}

#endif /*BERRYOSGIOBJECTLIST_H_*/
