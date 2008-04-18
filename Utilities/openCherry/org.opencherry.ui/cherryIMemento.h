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

#ifndef CHERRYIMEMENTO_H_
#define CHERRYIMEMENTO_H_

#include <org.opencherry.osgi/cherryDll.h>

#include <string>

namespace cherry {

struct CHERRY_API IMemento
{
  virtual int GetInt(std::string key) = 0;
  
  virtual ~IMemento() {};
};

} // namespace cherry

#endif /*CHERRYIMEMENTO_H_*/
