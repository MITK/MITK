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

#ifndef CHERRYSAMPLEAPPLICATION_H_
#define CHERRYSAMPLEAPPLICATION_H_

#include "org.opencherry.application/cherryIApplication.h"

namespace cherry {

class CHERRY_API QtWorkbenchApplication : public IApplication
{
public:
  int Run();
};

}

#endif /*CHERRYSAMPLEAPPLICATION_H_*/
