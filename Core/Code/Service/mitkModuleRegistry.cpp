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

#include "mitkModuleRegistry.h"

#include "mitkModule.h"
#include "mitkModuleInfo.h"
#include "mitkModuleContext.h"
#include "mitkModuleActivator.h"
#include "mitkCoreModuleContext_p.h"
#include "mitkStaticInit.h"

#include <itkSimpleFastMutexLock.h>
#include <itkMutexLockHolder.h>

#include <cassert>

namespace mitk {

#ifdef MITK_HAS_UNORDERED_MAP_H
typedef std::unordered_map<long, Module*> ModuleMap;
#else
typedef itk::hash_map<long, Module*> ModuleMap;
#endif

MITK_GLOBAL_STATIC(CoreModuleContext, coreModuleContext)

template<typename T>
struct ModuleDeleter
{
  void operator()(GlobalStatic<T>& globalStatic) const
  {
    ModuleMap* moduleMap = globalStatic.pointer;
    for (ModuleMap::const_iterator i = moduleMap->begin();
         i != moduleMap->end(); ++i)
    {
      delete i->second;
    }
    DefaultGlobalStaticDeleter<T> defaultDeleter;
    defaultDeleter(globalStatic);
  }
};

/**
 * Table of all installed modules in this framework.
 * Key is the module id.
 */
MITK_GLOBAL_STATIC_WITH_DELETER(ModuleMap, modules, ModuleDeleter)


typedef itk::SimpleFastMutexLock MutexType;
typedef itk::MutexLockHolder<MutexType> MutexLocker;

/**
 * Lock for protecting the modules object
 */
MITK_GLOBAL_STATIC(MutexType, modulesLock)

/**
 * Lock for protecting the register count
 */
MITK_GLOBAL_STATIC(MutexType, countLock)


void ModuleRegistry::Register(ModuleInfo* info)
{
  static long regCount = 0;
  if (info->id > 0)
  {
    // The module was already registered
    Module* module = 0;
    {
      MutexLocker lock(*modulesLock());
      module = modules()->operator[](info->id);
      assert(module != 0);
    }
    module->Start();
  }
  else
  {
    Module* module = 0;
    // check if the module is reloaded
    {
      MutexLocker lock(*modulesLock());
      ModuleMap* map = modules();
      for (ModuleMap::const_iterator i = map->begin();
           i != map->end(); ++i)
      {
        if (i->second->GetLocation() == info->location)
        {
          module = i->second;
          info->id = module->GetModuleId();
        }
      }
    }

    if (!module)
    {
      module = new Module();
      countLock()->Lock();
      info->id = ++regCount;
      countLock()->Unlock();

      module->Init(coreModuleContext(), info);

      MutexLocker lock(*modulesLock());
      ModuleMap* map = modules();
      map->insert(std::make_pair(info->id, module));
    }
    else
    {
      module->Init(coreModuleContext(), info);
    }

    module->Start();
  }
}

void ModuleRegistry::UnRegister(const ModuleInfo* info)
{
  // If we are unregistering the Mitk module, we just call
  // the module activators Unload() method (if there is a
  // module activator). Since we cannot be sure that the
  // ModuleContext for the Mitk library is still valid, we
  // just pass a null-pointer. Using the module context during
  // static deinitalization time of the Mitk library makes
  // no sense anyway.
  if (info->id == 1 && info->activatorHook)
  {
    info->activatorHook()->Unload(0);
    return;
  }

  Module* curr = 0;
  {
    MutexLocker lock(*modulesLock());
    curr = modules()->operator[](info->id);
    assert(curr != 0);
  }
  curr->Stop();
  curr->Uninit();
}

Module* ModuleRegistry::GetModule(long id)
{
  MutexLocker lock(*modulesLock());

  ModuleMap::const_iterator iter = modules()->find(id);
  if (iter != modules()->end())
  {
    return iter->second;
  }
  return 0;
}

Module* ModuleRegistry::GetModule(const std::string& name)
{
  MutexLocker lock(*modulesLock());

  ModuleMap::const_iterator iter = modules()->begin();
  ModuleMap::const_iterator iterEnd = modules()->end();
  for (; iter != iterEnd; ++iter)
  {
    if (iter->second->GetName() == name)
    {
      return iter->second;
    }
  }

  return 0;
}

void ModuleRegistry::GetModules(std::vector<Module*>& m)
{
  MutexLocker lock(*modulesLock());

  ModuleMap* map = modules();
  ModuleMap::const_iterator iter = map->begin();
  ModuleMap::const_iterator iterEnd = map->end();
  for (; iter != iterEnd; ++iter)
  {
    m.push_back(iter->second);
  }
}

void ModuleRegistry::GetLoadedModules(std::vector<Module*>& m)
{
  MutexLocker lock(*modulesLock());

  ModuleMap::const_iterator iter = modules()->begin();
  ModuleMap::const_iterator iterEnd = modules()->end();
  for (; iter != iterEnd; ++iter)
  {
    if (iter->second->IsLoaded())
    {
      m.push_back(iter->second);
    }
  }
}

}
