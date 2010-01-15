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

#ifndef OSGI_FRAMEWORK_OBJECTVECTOR_H_
#define OSGI_FRAMEWORK_OBJECTVECTOR_H_

#include "osgi/framework/Macros.h"

#include <vector>

namespace osgi {

namespace framework {

template<typename T>
class ObjectVector : public Object, public std::vector<T>
{
public:
  osgiObjectMacro(ObjectVector<T>);

  ObjectVector()
  {

  }

  ObjectVector(const Self& other) : Object(), std::vector<T>(other)
  {

  }

  ObjectVector(const std::vector<T>& other) : Object(), std::vector<T>(other)
  {

  }

  Object::Pointer Clone() const
  {
    Object::Pointer clone(new Self(*this));
    return clone;
  }

  bool operator==(const Object* obj) const
  {
    if (const ObjectVector* other = dynamic_cast<const ObjectVector*>(obj))
      static_cast<const std::vector<T> &>(*this) == static_cast<const std::vector<T>& >(*other);

    return false;
  }
};

}
}

#endif /*OSGI_FRAMEWORK_OBJECTVECTOR_H_*/
