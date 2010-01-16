@PLUGIN_COPYRIGHT@

#include "@GENERATE_VIEW_CLASS@.h"

#include "berryPlatform.h"
#include "service/berryIExtensionPointService.h"
#include "event/berryPlatformEvents.h"

#include "Poco/Delegate.h"

#include <iostream>

namespace berry {

const std::string RuntimePlugin::@PLUGIN_ID@ = "org.blueberry.core.runtime";

void
RuntimePlugin::Start(IBundleContext::Pointer /*context*/)
{
}

}
