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

#ifndef CHERRYOSGISTRING_H_
#define CHERRYOSGISTRING_H_

#include "cherryMacros.h"

#include <string>

namespace cherry {

class CHERRY_API ObjectString : public std::string, public Object
{
public:
  cherryClassMacro(ObjectString);
  
  ObjectString() {}
  ObjectString(const std::string& s) : std::string(s) {}
};

}

#endif /*CHERRYOSGISTRING_H_*/
