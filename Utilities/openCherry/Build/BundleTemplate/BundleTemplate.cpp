/*=========================================================================
 
Program:   openCherry Platform
Language:  C++
Date:      $Date: 2008-06-12 19:58:42 +0200 (Do, 12 Jun 2008) $
Version:   $Revision: 14610 $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

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
