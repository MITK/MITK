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

#ifndef OSGI_FRAMEWORK_OBJECT_STRING_H_
#define OSGI_FRAMEWORK_OBJECT_STRING_H_

#include "osgi/framework/Macros.h"

#include <string>

namespace osgi {

namespace framework {

class ObjectString : public std::string, public Object
{
public:
  osgiObjectMacro(ObjectString);

  ObjectString() {}
  ObjectString(const Self& s) : std::string(s), Object() {}
  ObjectString(const std::string& s) : std::string(s) {}

  Object::Pointer Clone() const
  {
    Object::Pointer clone(new Self(*this));
    return clone;
  }

  ~ObjectString() {}
};

}
}

#endif /*OSGI_FRAMEWORK_OBJECT_STRING_H_*/
