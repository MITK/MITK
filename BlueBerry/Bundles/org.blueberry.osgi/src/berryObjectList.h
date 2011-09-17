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

#ifndef BERRYOSGIOBJECTLIST_H_
#define BERRYOSGIOBJECTLIST_H_

#include "berryMacros.h"

#include <list>

namespace berry {

template<typename T>
class ObjectList : public Object, public std::list<T>
{
public:
  berryObjectMacro(ObjectList<T>);
};

}

#endif /*BERRYOSGIOBJECTLIST_H_*/
