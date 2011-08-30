/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
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

#ifndef MITKMODULEEVENT_H
#define MITKMODULEEVENT_H

#include "mitkSharedData.h"

namespace mitk {

class Module;
class ModuleEventData;

/**
 * \ingroup MicroServices
 *
 * An event from the Micro Services framework describing a module lifecycle change.
 * <p>
 * <code>ModuleEvent</code> objects are delivered to listeners connected
 * via ModuleContext::AddModuleListener() when a change
 * occurs in a modules's lifecycle. A type code is used to identify
 * the event type for future extendability.
 *
 * @see ModuleContext#connectModuleListener
 */
class MITK_CORE_EXPORT ModuleEvent
{

  SharedDataPointer<ModuleEventData> d;

public:

  enum Type {

    /**
     * The module has been loaded.
     * <p>
     * The module's
     * \link ModuleActivator::Load(ModuleContext*) ModuleActivator Load\endlink method
     * has been executed.
     */
    LOADED,

    /**
     * The module has been unloaded.
     * <p>
     * The module's
     * \link ModuleActivator::Unload(ModuleContext*) ModuleActivator Unload\endlink method
     * has been executed.
     */
    UNLOADED,

    /**
     * The module is about to be loaded.
     * <p>
     * The module's
     * \link ModuleActivator::Load(ModuleContext*) ModuleActivator Load\endlink method
     * is about to be called.
     */
    LOADING,

    /**
     * The module is about to be unloaded.
     * <p>
     * The module's
     * \link ModuleActivator::Unload(ModuleContext*) ModuleActivator Unload\endlink method
     * is about to be called.
     */
    UNLOADING

  };

  /**
   * Creates an invalid instance.
   */
  ModuleEvent();

  ~ModuleEvent();

  /**
   * Can be used to check if this ModuleEvent instance is valid,
   * or if it has been constructed using the default constructor.
   *
   * @return <code>true</code> if this event object is valid,
   *         <code>false</code> otherwise.
   */
  bool IsNull() const;

  /**
   * Creates a module event of the specified type.
   *
   * @param type The event type.
   * @param module The module which had a lifecycle change.
   */
  ModuleEvent(Type type, Module* module);

  ModuleEvent(const ModuleEvent& other);

  ModuleEvent& operator=(const ModuleEvent& other);

  /**
   * Returns the module which had a lifecycle change.
   *
   * @return The module that had a change occur in its lifecycle.
   */
  Module* GetModule() const;

  /**
   * Returns the type of lifecyle event. The type values are:
   * <ul>
   * <li>{@link #LOADING}
   * <li>{@link #LOADED}
   * <li>{@link #UNLOADING}
   * <li>{@link #UNLOADED}
   * </ul>
   *
   * @return The type of lifecycle event.
   */
  Type GetType() const;

};

}

/**
 * \ingroup MicroServices
 * @{
 */
MITK_CORE_EXPORT std::ostream& operator<<(std::ostream& os, mitk::ModuleEvent::Type eventType);
MITK_CORE_EXPORT std::ostream& operator<<(std::ostream& os, const mitk::ModuleEvent& event);
/** @}*/

#endif // MITKMODULEEVENT_H
