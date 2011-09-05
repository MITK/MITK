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

#ifndef MITKMODULEPRIVATE_H
#define MITKMODULEPRIVATE_H

#include "mitkModuleRegistry.h"
#include "mitkModuleVersion.h"
#include "mitkModuleInfo.h"

#include "mitkAtomicInt.h"

namespace mitk {

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

  std::vector<long> requiresIds;

  std::vector<std::string> requiresLibs;
  std::vector<std::string> requiresPackages;

  /**
   * Module version
   */
  ModuleVersion version;

  ModuleInfo info;

  /**
   * ModuleContext for the module
   */
  ModuleContext* moduleContext;

  ModuleActivator* moduleActivator;

  std::map<std::string, std::string> properties;

  Module* const q;

private:

  static AtomicInt idCounter;

};

}

#endif // MITKMODULEPRIVATE_H
