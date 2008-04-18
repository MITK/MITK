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

#ifndef CHERRYOSGIOBJECTVECTOR_H_
#define CHERRYOSGIOBJECTVECTOR_H_

#include "cherryMacros.h"

#include <vector>

namespace cherry {

class CHERRY_API ObjectVector : public Object, public std::vector<Object::Pointer>
{
public:
  cherryClassMacro(ObjectVector);
};

}

#endif /*CHERRYOSGIOBJECTVECTOR_H_*/
