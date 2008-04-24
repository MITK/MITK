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

#ifndef CHERRYIBUNDLEACTIVATOR_
#define CHERRYIBUNDLEACTIVATOR_

#include "cherryDll.h"

#include "cherrySmartPointer.h"

namespace cherry {

struct IBundleContext;

struct CHERRY_API IBundleActivator
{
  virtual void Start(SmartPointer<IBundleContext> context) = 0;
  virtual void Stop(SmartPointer<IBundleContext> context) = 0;
  
  virtual ~IBundleActivator() {};
};

}  // namespace cherry

#endif /*CHERRYIBUNDLEACTIVATOR_*/
