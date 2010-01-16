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

#ifndef BERRYRUNTIMEACTIVATOR_H_
#define BERRYRUNTIMEACTIVATOR_H_

#include <berryPlugin.h>
#include <berryIBundleContext.h>
#include <berryPlatformEvent.h>

#include "berryRuntimeDll.h"
#include "internal/berryPreferencesService.h"

namespace berry {

class BERRY_RUNTIME RuntimePlugin : public Plugin
{
public:
  
  static const std::string PLUGIN_ID;
  
  void Start(IBundleContext::Pointer context);
  void Stop(IBundleContext::Pointer context);
protected:
  PreferencesService::Pointer m_PreferencesService;
};

}

#endif /*BERRYRUNTIMEACTIVATOR_H_*/
