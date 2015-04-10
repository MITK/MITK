/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef BERRYDEBUGOPTIONSLISTENER_H
#define BERRYDEBUGOPTIONSLISTENER_H

#include <org_blueberry_core_runtime_Export.h>

#include <QtPlugin>

namespace berry {

class DebugOptions;

/**
 * A debug options listener is notified whenever one of its plug-in option-path entries is
 * changed.  A listener is registered as a service using the DebugOptions#LISTENER_SYMBOLICNAME
 * service property to specify the symbolic name of the debug options listener.
 * <p>
 * The DebugOptionsListener#OptionsChanged(const DebugOptions&) method will automatically
 * be called upon registration of the debug options listener service. This allows the
 * listener to obtain the initial debug options. This initial call to the listener
 * will happen even if debug is not enabled at the time of registration
 * DebugOptions#IsDebugEnabled() will return false in this case).
 * </p>
 * A debug options listener allows a plug-in to cache trace option values in boolean fields for performance
 * and code cleanliness. For example:
 * <pre>
 * class Activator : ctkPluginActivator, DebugOptionsListener
 * {
 *   static bool DEBUG = false;
 *
 *   void Start(ctkPluginContext* context)
 *   {
 *     ctkDictionary props;
 *     props.insert(DebugOptions::LISTENER_SYMBOLICNAME, "com.mycompany.mybundle");
 *     context->registerService<DebugOptionsListener>(this, props);
 *   }
 *
 *   void OptionsChanged(const DebugOptions& options)
 *   {
 *     DEBUG = options->GetBooleanOption("com.mycompany.mybundle/debug", false);
 *   }
 *
 *   void DoSomeWork()
 *   {
 *     if (DEBUG) BERRY_INFO << "foo";
 *   }
 *   ...
 * }
 * </pre>
 */
struct org_blueberry_core_runtime_EXPORT DebugOptionsListener
{
  virtual ~DebugOptionsListener();

  /**
   * Notifies this listener that an option-path for its plug-in has changed.
   * This method is also called initially by the DebugOptions implementation
   * when the listener is registered as a service.  This allows the listener
   * to obtain the initial set of debug options without the need to
   * acquire the debug options service.
   * @param options a reference to the DebugOptions
   */
  virtual void OptionsChanged(const DebugOptions& options) = 0;
};

}

Q_DECLARE_INTERFACE(berry::DebugOptionsListener, "org.blueberry.DebugOptionsListener")

#endif // BERRYDEBUGOPTIONSLISTENER_H
