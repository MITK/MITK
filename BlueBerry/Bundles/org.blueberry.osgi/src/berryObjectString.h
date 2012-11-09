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

#ifndef BERRYOSGISTRING_H_
#define BERRYOSGISTRING_H_

#include "berryMacros.h"

#include <string>

#ifdef org_blueberry_osgi_EXPORTS
  #define EXPORT_TEMPLATE
#else
  #define EXPORT_TEMPLATE extern
#endif

namespace berry {

//EXPORT_TEMPLATE template class BERRY_OSGI std::basic_string<char, std::char_traits<char>, std::allocator<char> >;

class ObjectString : public std::string, public Object
{
public:
  berryObjectMacro(ObjectString);

  ObjectString() {}
  ObjectString(const std::string& s) : std::string(s) {}

  ~ObjectString() {}
};

}

#endif /*BERRYOSGISTRING_H_*/
