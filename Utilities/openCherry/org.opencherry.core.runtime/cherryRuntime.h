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

#ifndef CHERRYRUNTIME_H_
#define CHERRYRUNTIME_H_

#include <string>

#include <org.opencherry.osgi/cherryDll.h>

namespace cherry {

struct CHERRY_API Runtime
{
  static const std::string ADAPTER_SERVICE_ID;
};

}

#endif /*CHERRYRUNTIME_H_*/
