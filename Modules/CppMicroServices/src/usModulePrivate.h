/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef USMODULEPRIVATE_H
#define USMODULEPRIVATE_H

#include <map>
#include <list>

#include <usModuleRegistry.h>
#include <usModuleVersion.h>
#include <usModuleInfo.h>
#include "usModuleManifest_p.h"
#include "usModuleResourceContainer_p.h"

#include <usAtomicInt_p.h>

namespace us {

class CoreModuleContext;
class ModuleContext;
struct ModuleActivator;

/**
 * \brief Internal implementation data for a Module.
 *
 * This class is not part of the public API.
 *
 * \ingroup MicroServices
 * \sa Module ModuleContext ModuleInfo
 */
class ModulePrivate {

public:

  /**
   * \brief Construct a new module based on a ModuleInfo object.
   *
   * \param[in] qq The public Module instance (q-pointer).
   * \param[in] coreCtx The shared core module context.
   * \param[in] info Module metadata used for initialization.
   */
  ModulePrivate(Module* qq, CoreModuleContext* coreCtx, ModuleInfo* info);

  /** \brief Destructor. */
  virtual ~ModulePrivate();

  /** \brief Remove all registered resources for this module. */
  void RemoveModuleResources();

  CoreModuleContext* const coreCtx; ///< \brief The shared core module context.

  /** \brief Module version. */
  ModuleVersion version;

  ModuleInfo info; ///< \brief Module metadata.

  ModuleResourceContainer resourceContainer; ///< \brief Container for embedded resources.

  /** \brief ModuleContext for the module. */
  ModuleContext* moduleContext;

  ModuleActivator* moduleActivator; ///< \brief Optional activator for start/stop callbacks.

  ModuleManifest moduleManifest; ///< \brief Parsed manifest properties.

  std::string baseStoragePath; ///< \brief Base persistent storage path.
  std::string storagePath; ///< \brief Module-specific persistent storage path.

  Module* const q; ///< \brief Pointer to the public Module (q-pointer pattern).

private:

  /** \brief Load and initialize embedded module resources. */
  void InitializeResources();

  static AtomicInt idCounter; ///< \brief Global module ID counter.

  // purposely not implemented
  ModulePrivate(const ModulePrivate&);
  ModulePrivate& operator=(const ModulePrivate&);

};

}

#endif // USMODULEPRIVATE_H
