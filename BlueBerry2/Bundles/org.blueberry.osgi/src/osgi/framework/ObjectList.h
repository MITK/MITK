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

#ifndef OSGI_FRAMEWORK_OBJECTLIST_H_
#define OSGI_FRAMEWORK_OBJECTLIST_H_

#include "osgi/framework/Macros.h"

#include <list>

namespace osgi {

namespace framework {

template<typename T>
class ObjectList : public Object, public std::list<T>
{
public:
  osgiObjectMacro(ObjectList<T>);
};

}
}

#endif /*OSGI_FRAMEWORK_OBJECTLIST_H_*/
