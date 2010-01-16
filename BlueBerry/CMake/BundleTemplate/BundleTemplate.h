@PLUGIN_COPYRIGHT@

#ifndef (@GENERATE_VIEW_CLASS@_H_INCLUDED)
#define @GENERATE_VIEW_CLASS@_H_INCLUDED

#include <berryPlugin.h>
#include <berryIBundleContext.h>
#include <event/berryPlatformEvent.h>

#include "berryRuntimeDll.h"

namespace berry {

class BERRY_RUNTIME @GENERATE_VIEW_CLASS@ : public Plugin
{
public:

  static const std::string @PLUGIN_ID@;

  void Start(IBundleContext::Pointer context);

};

}

#endif /*@GENERATE_VIEW_CLASS@_H_INCLUDED*/
