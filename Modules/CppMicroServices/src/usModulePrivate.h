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
 * \ingroup MicroServices
 */
class ModulePrivate {

public:

  /**
   * Construct a new module based on a ModuleInfo object.
   */
  ModulePrivate(Module* qq, CoreModuleContext* coreCtx, ModuleInfo* info);

  virtual ~ModulePrivate();

  void RemoveModuleResources();

  CoreModuleContext* const coreCtx;

  /**
   * Module version
   */
  ModuleVersion version;

  ModuleInfo info;

  ModuleResourceContainer resourceContainer;

  /**
   * ModuleContext for the module
   */
  ModuleContext* moduleContext;

  ModuleActivator* moduleActivator;

  ModuleManifest moduleManifest;

  std::string baseStoragePath;
  std::string storagePath;

  Module* const q;

private:

  void InitializeResources();

  static AtomicInt idCounter;

  // purposely not implemented
  ModulePrivate(const ModulePrivate&);
  ModulePrivate& operator=(const ModulePrivate&);

};

}

#endif // USMODULEPRIVATE_H
