#ifndef MITKCORESERVICESPLUGIN_H_
#define MITKCORESERVICESPLUGIN_H_

#include <cherryPlugin.h>
#include <cherryIBundleContext.h>

#include "mitkCoreServicesPluginDll.h"

namespace mitk
{

class MITK_CORE_SERVICES_PLUGIN CoreServicesPlugin : public cherry::Plugin
{
public:
  
  static const std::string PLUGIN_ID;
  
  void Start(cherry::IBundleContext::Pointer context);
  
};

}

#endif /*MITKCORESERVICESPLUGIN_H_*/
