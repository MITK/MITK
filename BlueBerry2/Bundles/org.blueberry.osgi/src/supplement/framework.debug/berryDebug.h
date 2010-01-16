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
 
 ========================================================================*/

#ifndef BERRYDEBUG_H_
#define BERRYDEBUG_H_

#include <string>

namespace berry
{
namespace osgi
{

/**
 * This class has debug constants which can be used by the Framework implementation
 * and Adaptor implementations
 * @since 3.1
 */
struct Debug
{
  /**
   * DEBUG flag.  If set to false then the debug statements will be optimized out during compilation.
   */
  static const bool DEBUG; // = true;

  /**
   * Indicates if tracing is enabled
   */
  static bool DEBUG_ENABLED; // = false;

  /**
   * General debug flag.
   */
  static bool DEBUG_GENERAL; // = false; // "debug"
  /**
   * Bundle time debug flag.
   */
  static bool DEBUG_BUNDLE_TIME; // = false; //"debug.bundleTime"
  /**
   * Loader debug flag.
   */
  static bool DEBUG_LOADER; // = false; // "debug.loader"
  /**
   * Events debug flag.
   */
  static bool DEBUG_EVENTS; // = false; // "debug.events"
  /**
   * Services debug flag.
   */
  static bool DEBUG_SERVICES; // = false; // "debug.services"
  /**
   * Packages debug flag.
   */
  // TODO remove this or use it somewhere
  static bool DEBUG_PACKAGES; // = false; // "debug.packages"
  /**
   * Manifest debug flag.
   */
  static bool DEBUG_MANIFEST; // = false; // "debug.manifest"
  /**
   * Filter debug flag.
   */
  static bool DEBUG_FILTER; // = false; // "debug.filter"
  /**
   * Security debug flag.
   */
  static bool DEBUG_SECURITY; // = false; // "debug.security"
  /**
   * Start level debug flag.
   */
  static bool DEBUG_STARTLEVEL; // = false; // "debug.startlevel"
  /**
   * PackageAdmin debug flag.
   */
  static bool DEBUG_PACKAGEADMIN; // = false; // "debug.packageadmin"
  /**
   * PackageAdmin timing debug flag.
   */
  // TODO remove this or use it somewhere
  static bool DEBUG_PACKAGEADMIN_TIMING; // = false; //"debug.packageadmin/timing"
  /**
   * Message debug flag.
   */
  static bool DEBUG_MESSAGE_BUNDLES; // = false; //"/debug/messageBundles"
  /**
   * Monitor activation debug flag.
   */
  static bool MONITOR_ACTIVATION; // = false; // "monitor/bundles"

  /**
   * Base debug option key (org.blueberry.osgi).
   */
  static const std::string BLUEBERRY_OSGI; // = "org.blueberry.osgi"; //$NON-NLS-1$
  /**
   * General Debug option key.
   */
  static const std::string OPTION_DEBUG_GENERAL; // = BLUEBERRY_OSGI + "/debug"; //$NON-NLS-1$
  /**
   * Bundle time Debug option key.
   */
  // TODO remove this or use it somewhere
  static const std::string OPTION_DEBUG_BUNDLE_TIME; // = BLUEBERRY_OSGI + "/debug/bundleTime"; //$NON-NLS-1$
  /**
   * Loader Debug option key.
   */
  static const std::string OPTION_DEBUG_LOADER; // = BLUEBERRY_OSGI + "/debug/loader"; //$NON-NLS-1$
  /**
   * Events Debug option key.
   */
  static const std::string OPTION_DEBUG_EVENTS; // = BLUEBERRY_OSGI + "/debug/events"; //$NON-NLS-1$
  /**
   * Services Debug option key.
   */
  static const std::string OPTION_DEBUG_SERVICES; // = BLUEBERRY_OSGI + "/debug/services"; //$NON-NLS-1$
  /**
   * Packages Debug option key.
   */
  static const std::string OPTION_DEBUG_PACKAGES; // = BLUEBERRY_OSGI + "/debug/packages"; //$NON-NLS-1$
  /**
   * Manifest Debug option key.
   */
  static const std::string OPTION_DEBUG_MANIFEST; // = BLUEBERRY_OSGI + "/debug/manifest"; //$NON-NLS-1$
  /**
   * Filter Debug option key.
   */
  static const std::string OPTION_DEBUG_FILTER; // = BLUEBERRY_OSGI + "/debug/filter"; //$NON-NLS-1$
  /**
   * Security Debug option key.
   */
  static const std::string OPTION_DEBUG_SECURITY; // = BLUEBERRY_OSGI + "/debug/security"; //$NON-NLS-1$
  /**
   * Start level Debug option key.
   */
  static const std::string OPTION_DEBUG_STARTLEVEL; // = BLUEBERRY_OSGI + "/debug/startlevel"; //$NON-NLS-1$
  /**
   * PackageAdmin Debug option key.
   */
  static const std::string OPTION_DEBUG_PACKAGEADMIN; // = BLUEBERRY_OSGI + "/debug/packageadmin"; //$NON-NLS-1$
  /**
   * PackageAdmin timing Debug option key.
   */
  static const std::string OPTION_DEBUG_PACKAGEADMIN_TIMING; // = BLUEBERRY_OSGI + "/debug/packageadmin/timing"; //$NON-NLS-1$
  /**
   * Monitor activation Debug option key.
   */
  static const std::string OPTION_MONITOR_ACTIVATION; // = BLUEBERRY_OSGI + "/monitor/activation"; //$NON-NLS-1$
  /**
   * Message bundles Debug option key.
   */
  static const std::string OPTION_DEBUG_MESSAGE_BUNDLES; // = BLUEBERRY_OSGI + "/debug/messageBundles"; //$NON-NLS-1$

  /**
   * Prints t to the PrintStream
   * @param t
   */
  static void PrintStackTrace();

private:

  struct Initializer
  {

    Initializer();
  };

  static Initializer initializer;

};

}
}

#endif /* BERRYDEBUG_H_ */
