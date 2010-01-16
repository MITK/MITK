/*=========================================================================
 
 Program:   BlueBerry Platform
 Language:  C++
 Date:      $Date$
 Version:   $Revision$
 
 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.
 
 =========================================================================*/

#include "berryDebug.h"

#include <internal/supplement/berryStackTrace.h>

#include <iostream>

namespace berry
{
namespace osgi
{

 const bool Debug::DEBUG = true;

 bool Debug::DEBUG_ENABLED = false;
 bool Debug::DEBUG_GENERAL = false; // "debug"
 bool Debug::DEBUG_BUNDLE_TIME = false; //"debug.bundleTime"
 bool Debug::DEBUG_LOADER = false; // "debug.loader"
 bool Debug::DEBUG_EVENTS = false; // "debug.events"
 bool Debug::DEBUG_SERVICES = false; // "debug.services"
// TODO remove this or use it somewhere
 bool Debug::DEBUG_PACKAGES = false; // "debug.packages"
 bool Debug::DEBUG_MANIFEST = false; // "debug.manifest"
 bool Debug::DEBUG_FILTER = false; // "debug.filter"
 bool Debug::DEBUG_SECURITY = false; // "debug.security"
 bool Debug::DEBUG_STARTLEVEL = false; // "debug.startlevel"
 bool Debug::DEBUG_PACKAGEADMIN = false; // "debug.packageadmin"
// TODO remove this or use it somewhere
 bool Debug::DEBUG_PACKAGEADMIN_TIMING = false; //"debug.packageadmin/timing"
 bool Debug::DEBUG_MESSAGE_BUNDLES = false; //"/debug/messageBundles"
 bool Debug::MONITOR_ACTIVATION = false; // "monitor/bundles"

 const std::string Debug::BLUEBERRY_OSGI = "org.blueberry.osgi"; //$NON-NLS-1$
 const std::string Debug::OPTION_DEBUG_GENERAL = BLUEBERRY_OSGI + "/debug"; //$NON-NLS-1$
// TODO remove this or use it somewhere
 const std::string Debug::OPTION_DEBUG_BUNDLE_TIME = BLUEBERRY_OSGI
    + "/debug/bundleTime"; //$NON-NLS-1$
 const std::string Debug::OPTION_DEBUG_LOADER = BLUEBERRY_OSGI
    + "/debug/loader"; //$NON-NLS-1$
 const std::string Debug::OPTION_DEBUG_EVENTS = BLUEBERRY_OSGI
    + "/debug/events"; //$NON-NLS-1$
 const std::string Debug::OPTION_DEBUG_SERVICES = BLUEBERRY_OSGI
    + "/debug/services"; //$NON-NLS-1$
 const std::string Debug::OPTION_DEBUG_PACKAGES = BLUEBERRY_OSGI
    + "/debug/packages"; //$NON-NLS-1$
 const std::string Debug::OPTION_DEBUG_MANIFEST = BLUEBERRY_OSGI
    + "/debug/manifest"; //$NON-NLS-1$
 const std::string Debug::OPTION_DEBUG_FILTER = BLUEBERRY_OSGI
    + "/debug/filter"; //$NON-NLS-1$
 const std::string Debug::OPTION_DEBUG_SECURITY = BLUEBERRY_OSGI
    + "/debug/security"; //$NON-NLS-1$
 const std::string Debug::OPTION_DEBUG_STARTLEVEL = BLUEBERRY_OSGI
    + "/debug/startlevel"; //$NON-NLS-1$
 const std::string Debug::OPTION_DEBUG_PACKAGEADMIN = BLUEBERRY_OSGI
    + "/debug/packageadmin"; //$NON-NLS-1$
 const std::string Debug::OPTION_DEBUG_PACKAGEADMIN_TIMING = BLUEBERRY_OSGI
    + "/debug/packageadmin/timing"; //$NON-NLS-1$
 const std::string Debug::OPTION_MONITOR_ACTIVATION = BLUEBERRY_OSGI
    + "/monitor/activation"; //$NON-NLS-1$
 const std::string Debug::OPTION_DEBUG_MESSAGE_BUNDLES = BLUEBERRY_OSGI
    + "/debug/messageBundles"; //$NON-NLS-1$

 void Debug::PrintStackTrace()
{
  StackTrace();
}

Debug::Initializer::Initializer()
{

  //TODO get the proper program name
  StackTraceInit("ExtApp", -1);

  //TODO implement FrameworkDebugOptions
  //    FrameworkDebugOptions dbgOptions = FrameworkDebugOptions.getDefault();
  //    if (dbgOptions ! = null) {
  //      DEBUG_ENABLED  = dbgOptions.isDebugEnabled();
  //      DEBUG_GENERAL  = dbgOptions.getBooleanOption(OPTION_DEBUG_GENERAL, false);
  //      DEBUG_BUNDLE_TIME  = dbgOptions.getBooleanOption(OPTION_DEBUG_BUNDLE_TIME, false) || dbgOptions.getBooleanOption("org.eclipse.core.runtime/timing/startup", false); //$NON-NLS-1$
  //      DEBUG_LOADER  = dbgOptions.getBooleanOption(OPTION_DEBUG_LOADER, false);
  //      DEBUG_EVENTS  = dbgOptions.getBooleanOption(OPTION_DEBUG_EVENTS, false);
  //      DEBUG_SERVICES  = dbgOptions.getBooleanOption(OPTION_DEBUG_SERVICES, false);
  //      DEBUG_PACKAGES  = dbgOptions.getBooleanOption(OPTION_DEBUG_PACKAGES, false);
  //      DEBUG_MANIFEST  = dbgOptions.getBooleanOption(OPTION_DEBUG_MANIFEST, false);
  //      DEBUG_FILTER  = dbgOptions.getBooleanOption(OPTION_DEBUG_FILTER, false);
  //      DEBUG_SECURITY  = dbgOptions.getBooleanOption(OPTION_DEBUG_SECURITY, false);
  //      DEBUG_STARTLEVEL  = dbgOptions.getBooleanOption(OPTION_DEBUG_STARTLEVEL, false);
  //      DEBUG_PACKAGEADMIN  = dbgOptions.getBooleanOption(OPTION_DEBUG_PACKAGEADMIN, false);
  //      DEBUG_PACKAGEADMIN_TIMING  = dbgOptions.getBooleanOption(OPTION_DEBUG_PACKAGEADMIN_TIMING, false) || dbgOptions.getBooleanOption("org.eclipse.core.runtime/debug", false); //$NON-NLS-1$
  //      DEBUG_MESSAGE_BUNDLES  = dbgOptions.getBooleanOption(OPTION_DEBUG_MESSAGE_BUNDLES, false);
  //      MONITOR_ACTIVATION  = dbgOptions.getBooleanOption(OPTION_MONITOR_ACTIVATION, false);
  //    }
}

Debug::Initializer Debug::initializer = Debug::Initializer();

}
}
