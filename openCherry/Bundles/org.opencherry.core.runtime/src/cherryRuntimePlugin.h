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

#ifndef CHERRYRUNTIMEACTIVATOR_H_
#define CHERRYRUNTIMEACTIVATOR_H_

#include <cherryPlugin.h>
#include <cherryIBundleContext.h>
#include <cherryPlatformEvent.h>

#include "cherryRuntimeDll.h"

namespace cherry {

class CHERRY_RUNTIME RuntimePlugin : public Plugin
{
public:
  
  static const std::string PLUGIN_ID;
  
  void Start(IBundleContext::Pointer context);
  
};

}

#endif /*CHERRYRUNTIMEACTIVATOR_H_*/
