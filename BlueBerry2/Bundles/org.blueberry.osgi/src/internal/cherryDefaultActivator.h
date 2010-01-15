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

#ifndef CHERRYDEFAULTACTIVATOR_H_
#define CHERRYDEFAULTACTIVATOR_H_

#include "osgi/framework/IBundleActivator.h"
#include "osgi/framework/IBundleContext.h"

namespace cherry {

class DefaultActivator : public IBundleActivator
{
  void Start(IBundleContext::Pointer context);
  void Stop(IBundleContext::Pointer context);
};

}

#endif /*CHERRYDEFAULTACTIVATOR_H_*/
