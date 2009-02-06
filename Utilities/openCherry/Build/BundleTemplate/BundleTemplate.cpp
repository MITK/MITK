@PLUGIN_COPYRIGHT@

#include "@GENERATE_VIEW_CLASS@.h"

#include "cherryPlatform.h"
#include "service/cherryIExtensionPointService.h"
#include "event/cherryPlatformEvents.h"

#include "Poco/Delegate.h"

#include <iostream>

namespace cherry {

const std::string RuntimePlugin::@PLUGIN_ID@ = "org.opencherry.core.runtime";

void
RuntimePlugin::Start(IBundleContext::Pointer /*context*/)
{
}

}
