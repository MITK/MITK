/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center, 
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without 
even the implied warranty of MERCHANTABILITY or FITNESS FOR 
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef MITKCORESERVICESPLUGIN_H_
#define MITKCORESERVICESPLUGIN_H_

#include <berryPlugin.h>
#include <berryIBundleContext.h>

#include "mitkIDataStorageService.h"

namespace mitk
{

class org_mitk_core_services_Activator : public QObject, public ctkPluginActivator
{
  Q_OBJECT
  Q_INTERFACES(ctkPluginActivator)

public:
  
  static const std::string PLUGIN_ID;
  
  void start(ctkPluginContext* context);
  void stop(ctkPluginContext* context);

private:

  mitk::IDataStorageService::Pointer dataStorageService;
  
};

typedef org_mitk_core_services_Activator PluginActivator;

}

#endif /*MITKCORESERVICESPLUGIN_H_*/
