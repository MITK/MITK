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

#ifndef BERRYOSGIOBJECTVECTOR_H_
#define BERRYOSGIOBJECTVECTOR_H_

#include "berryMacros.h"

#include <vector>

namespace berry {

template<typename T>
class ObjectVector : public Object, public std::vector<T>
{
public:
  berryObjectMacro(ObjectVector<T>);

  bool operator==(const Object* obj) const
  {
    if (const ObjectVector* other = dynamic_cast<const ObjectVector*>(obj))
      static_cast<const std::vector<T> &>(*this) == static_cast<const std::vector<T>& >(*other);

    return false;
  }
};

}

#endif /*BERRYOSGIOBJECTVECTOR_H_*/
