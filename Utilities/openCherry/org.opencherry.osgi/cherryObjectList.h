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

#ifndef CHERRYOSGIOBJECTLIST_H_
#define CHERRYOSGIOBJECTLIST_H_

#include "cherryMacros.h"

#include <list>

namespace cherry {

template<typename T>
class ObjectList : public Object, public std::list<T>
{
public:
  cherryClassMacro(ObjectList<T>);
};

}

#endif /*CHERRYOSGIOBJECTLIST_H_*/
