/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef USMODULEEVENT_H
#define USMODULEEVENT_H

#include <iostream>

#include <usSharedData.h>

namespace us {

class Module;
class ModuleEventData;

/**
 * \ingroup MicroServices
 *
 * \brief An event from the Micro Services framework describing a module lifecycle change.
 * <p>
 * <code>ModuleEvent</code> objects are delivered to listeners connected
 * via ModuleContext::AddModuleListener() when a change
 * occurs in a modules's lifecycle. A type code is used to identify
 * the event type for future extendability.
 *
 * \sa ModuleContext#AddModuleListener
 */
class MITKCPPMICROSERVICES_EXPORT ModuleEvent
{

  SharedDataPointer<ModuleEventData> d;

public:

  enum Type {

    /**
     * \brief The module has been loaded.
     * <p>
     * The module's
     * \link ModuleActivator::Load(ModuleContext*) ModuleActivator Load\endlink method
     * has been executed.
     */
    LOADED,

    /**
     * \brief The module has been unloaded.
     * <p>
     * The module's
     * \link ModuleActivator::Unload(ModuleContext*) ModuleActivator Unload\endlink method
     * has been executed.
     */
    UNLOADED,

    /**
     * \brief The module is about to be loaded.
     * <p>
     * The module's
     * \link ModuleActivator::Load(ModuleContext*) ModuleActivator Load\endlink method
     * is about to be called.
     */
    LOADING,

    /**
     * \brief The module is about to be unloaded.
     * <p>
     * The module's
     * \link ModuleActivator::Unload(ModuleContext*) ModuleActivator Unload\endlink method
     * is about to be called.
     */
    UNLOADING

  };

  /**
   * \brief Creates an invalid instance.
   */
  ModuleEvent();

  /** \brief Destructor. */
  ~ModuleEvent();

  /**
   * \brief Can be used to check if this ModuleEvent instance is valid,
   * or if it has been constructed using the default constructor.
   *
   * \return <code>true</code> if this event object is valid,
   *         <code>false</code> otherwise.
   */
  bool IsNull() const;

  /**
   * \brief Creates a module event of the specified type.
   *
   * \param[in] type The event type.
   * \param[in] module The module which had a lifecycle change.
   */
  ModuleEvent(Type type, Module* module);

  /** \brief Copy constructor.
   * \param[in] other The ModuleEvent to copy.
   */
  ModuleEvent(const ModuleEvent& other);

  /** \brief Copy assignment operator.
   * \param[in] other The ModuleEvent to assign from.
   * \return A reference to this object.
   */
  ModuleEvent& operator=(const ModuleEvent& other);

  /**
   * \brief Returns the module which had a lifecycle change.
   *
   * \return The module that had a change occur in its lifecycle.
   */
  Module* GetModule() const;

  /**
   * \brief Returns the type of lifecycle event. The type values are:
   * <ul>
   * <li>{@link #LOADING}
   * <li>{@link #LOADED}
   * <li>{@link #UNLOADING}
   * <li>{@link #UNLOADED}
   * </ul>
   *
   * \return The type of lifecycle event.
   */
  Type GetType() const;

};

/**
 * \ingroup MicroServices
 * \{
 */

/** \brief Stream output operator for ModuleEvent::Type.
 *
 * \param[in] os The output stream.
 * \param[in] eventType The module event type to write to the stream.
 * \return The output stream.
 */
MITKCPPMICROSERVICES_EXPORT std::ostream& operator<<(std::ostream& os, ModuleEvent::Type eventType);

/** \brief Stream output operator for ModuleEvent.
 *
 * \param[in] os The output stream.
 * \param[in] event The module event to write to the stream.
 * \return The output stream.
 */
MITKCPPMICROSERVICES_EXPORT std::ostream& operator<<(std::ostream& os, const ModuleEvent& event);

/** \}*/

}

#endif // USMODULEEVENT_H
