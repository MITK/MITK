#ifndef MITKCORESERVICESPLUGIN_H_
#define MITKCORESERVICESPLUGIN_H_

#include <cherryPlugin.h>
#include <cherryIBundleContext.h>

#include "mitkCorePluginDll.h"

namespace mitk
{

class MITK_CORE_PLUGIN CoreServicesPlugin : public cherry::Plugin
{
public:
  
  static const std::string PLUGIN_ID;
  
  void Start(cherry::IBundleContext::Pointer context);
  
};

}

#endif /*MITKCORESERVICESPLUGIN_H_*/
