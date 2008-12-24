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

#ifndef CHERRYSYSTEMBUNDLEACTIVATOR_H_
#define CHERRYSYSTEMBUNDLEACTIVATOR_H_

#include "../cherryIBundleActivator.h"

namespace cherry {

class SystemBundleActivator : public IBundleActivator
{
public:
  void Start(SmartPointer<IBundleContext> context);
  void Stop(SmartPointer<IBundleContext> context);
  
};

}

#endif /*CHERRYSYSTEMBUNDLEACTIVATOR_H_*/
