@PLUGIN_COPYRIGHT@

#include "Poco/ClassLibrary.h"

#include <cherryIViewPart.h>
#include "src/internal/@VIEW_CLASS_H@"


//********************  Views  **********************
POCO_BEGIN_NAMED_MANIFEST(cherryIViewPart, cherry::IViewPart)
  POCO_EXPORT_CLASS(@BUNDLE_NAMESPACE@::@VIEW_CLASS@)
POCO_END_MANIFEST
