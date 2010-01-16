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

#ifndef OSGI_FRAMEWORK_IBUNDLEACTIVATOR_
#define OSGI_FRAMEWORK_IBUNDLEACTIVATOR_

#include "berryOSGiDll.h"

#include "SmartPointer.h"

namespace osgi {

namespace framework {

struct IBundleContext;

struct BERRY_OSGI IBundleActivator
{
  virtual void Start(SmartPointer<IBundleContext> context) = 0;
  virtual void Stop(SmartPointer<IBundleContext> context) = 0;
  
  virtual ~IBundleActivator() {};
};

} }  // namespace

#endif /*OSGI_FRAMEWORK_IBUNDLEACTIVATOR_*/
