#ifndef MITKCORESERVICESPLUGIN_H_
#define MITKCORESERVICESPLUGIN_H_

#include <berryPlugin.h>
#include <berryIBundleContext.h>

#include "mitkCoreServicesPluginDll.h"

namespace mitk
{

class MITK_CORE_SERVICES_PLUGIN CoreServicesPlugin : public berry::Plugin
{
public:
  
  static const std::string PLUGIN_ID;
  
  void Start(berry::IBundleContext::Pointer context);
  
};

}

#endif /*MITKCORESERVICESPLUGIN_H_*/
