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
  berryObjectMacro(berry::ObjectList<T>)

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
