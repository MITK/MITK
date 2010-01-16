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

#ifndef BERRYIBUNDLEACTIVATOR_
#define BERRYIBUNDLEACTIVATOR_

#include "berryOSGiDll.h"

#include "berrySmartPointer.h"

namespace berry {

struct IBundleContext;

struct BERRY_OSGI IBundleActivator
{
  virtual void Start(SmartPointer<IBundleContext> context) = 0;
  virtual void Stop(SmartPointer<IBundleContext> context) = 0;
  
  virtual ~IBundleActivator() {};
};

}  // namespace berry

#endif /*BERRYIBUNDLEACTIVATOR_*/
