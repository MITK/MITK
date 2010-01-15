@PLUGIN_COPYRIGHT@

#ifndef (@GENERATE_VIEW_CLASS@_H_INCLUDED)
#define @GENERATE_VIEW_CLASS@_H_INCLUDED

#include <cherryPlugin.h>
#include <cherryIBundleContext.h>
#include <event/cherryPlatformEvent.h>

#include "cherryRuntimeDll.h"

namespace cherry {

class CHERRY_RUNTIME @GENERATE_VIEW_CLASS@ : public Plugin
{
public:

  static const std::string @PLUGIN_ID@;

  void Start(IBundleContext::Pointer context);

};

}

#endif /*@GENERATE_VIEW_CLASS@_H_INCLUDED*/
