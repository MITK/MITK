/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef USMODULEIMPORT_H
#define USMODULEIMPORT_H

#include <usGlobalConfig.h>

#include <string>

namespace us {

struct ModuleActivator;

}

/**
 * \ingroup MicroServices
 *
 * \brief Initialize a static module.
 *
 * \param _module_name The name of the module to initialize.
 *
 * This macro initializes the static module named \c _module_name.
 *
 * It the module provides an activator, use the #US_IMPORT_MODULE macro instead,
 * to ensure that the activator is called. Do not forget to actually link
 * the static module to the importing executable or shared library.
 *
 * \sa US_IMPORT_MODULE
 * \sa US_IMPORT_MODULE_RESOURCES
 * \sa \ref MicroServices_StaticModules
 */
#define US_INITIALIZE_STATIC_MODULE(_module_name)                          \
  extern "C" void _us_import_module_initializer_ ## _module_name();        \
  struct StaticModuleInitializer_ ## _module_name                          \
  {                                                                        \
    StaticModuleInitializer_ ## _module_name()                             \
    {                                                                      \
      _us_import_module_initializer_ ## _module_name();                    \
    }                                                                      \
  };                                                                       \
  static StaticModuleInitializer_ ## _module_name _InitializeModule_ ## _module_name;

/**
 * \ingroup MicroServices
 *
 * \brief Import a static module.
 *
 * \param _module_name The name of the module to import.
 *
 * This macro imports the static module named \c _module_name.
 *
 * Inserting this macro into your application's source code will allow you to make use of
 * a static module. It will initialize the static module and calls its
 * ModuleActivator. It the module does not provide an activator, use the
 * #US_INITIALIZE_STATIC_MODULE macro instead. Do not forget to actually link
 * the static module to the importing executable or shared library.
 *
 * Example:
 * \snippet uServices-staticmodules/main.cpp ImportStaticModuleIntoMain
 *
 * \sa US_INITIALIZE_STATIC_MODULE
 * \sa US_IMPORT_MODULE_RESOURCES
 * \sa \ref MicroServices_StaticModules
 */
#define US_IMPORT_MODULE(_module_name)                                     \
  US_INITIALIZE_STATIC_MODULE(_module_name)                                \
  extern "C" us::ModuleActivator* _us_module_activator_instance_ ## _module_name (); \
  void _dummy_reference_to_ ## _module_name ## _activator()                \
  {                                                                        \
    _us_module_activator_instance_ ## _module_name();                      \
  }

#endif // USMODULEREGISTRY_H
