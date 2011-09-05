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


#include "mitkModule.h"

#include "mitkModuleContext.h"
#include "mitkModuleActivator.h"
#include "mitkModulePrivate.h"
#include "mitkCoreModuleContext_p.h"

#include <mitkLogMacros.h>

namespace mitk {

const std::string& Module::PROP_ID()
{
  static const std::string s("module.id");
  return s;
}
const std::string& Module::PROP_NAME()
{
  static const std::string s("module.name");
  return s;
}
const std::string& Module::PROP_LOCATION()
{
  static const std::string s("module.location");
  return s;
}
const std::string& Module::PROP_MODULE_DEPENDS()
{
  static const std::string s("module.module_depends");
  return s;
}
const std::string& Module::PROP_LIB_DEPENDS()
{
  static const std::string s("module.lib_depends");
  return s;
}
const std::string& Module::PROP_PACKAGE_DEPENDS()
{
  static const std::string s("module.package_depends");
  return s;
}
const std::string& Module::PROP_VERSION()
{
  static const std::string s("module.version");
  return s;
}
const std::string& Module::PROP_QT()
{
  static const std::string s("module.qt");
  return s;
}

Module::Module()
: d(0)
{

}

Module::~Module()
{
  delete d;
}

void Module::Init(CoreModuleContext* coreCtx,
                  ModuleInfo* info)
{
  ModulePrivate* mp = new ModulePrivate(this, coreCtx, info);
  std::swap(mp, d);
  delete mp;
}

bool Module::IsLoaded() const
{
  return d->moduleContext != 0;
}

void Module::Start()
{

  if (d->moduleContext)
  {
    MITK_WARN << "Module " << d->info.name << " already started.";
    return;
  }

  d->moduleContext = new ModuleContext(this->d);

//  try
//  {
    d->coreCtx->listeners.ModuleChanged(ModuleEvent(ModuleEvent::LOADING, this));
    // try to get a ModuleActivator instance
    if (d->info.activatorHook)
    {
      try
      {
        d->moduleActivator = d->info.activatorHook();
      }
      catch (...)
      {
        MITK_ERROR << "Creating the module activator of " << d->info.name << " failed";
        throw;
      }

      d->moduleActivator->Load(d->moduleContext);
    }

    d->coreCtx->listeners.ModuleChanged(ModuleEvent(ModuleEvent::LOADED, this));
//  }
//  catch (...)
//  {
//    d->coreCtx->listeners.ModuleChanged(ModuleEvent(ModuleEvent::UNLOADING, this));
//    d->RemoveModuleResources();

//    delete d->moduleContext;
//    d->moduleContext = 0;

//    d->coreCtx->listeners.ModuleChanged(ModuleEvent(ModuleEvent::UNLOADED, this));
//    MITK_ERROR << "Calling the module activator Load() method of " << d->info.name << " failed!";
//    throw;
//  }
}

void Module::Stop()
{
  if (d->moduleContext == 0)
  {
    MITK_WARN << "Module " << d->info.name << " already stopped.";
    return;
  }

  try
  {
    d->coreCtx->listeners.ModuleChanged(ModuleEvent(ModuleEvent::UNLOADING, this));
    if (d->moduleActivator)
    {
      d->moduleActivator->Unload(d->moduleContext);
    }
  }
  catch (...)
  {
    MITK_ERROR << "Calling the module activator Unload() method of " << d->info.name << " failed!";

    try
    {
      d->RemoveModuleResources();
    }
    catch (...) {}

    delete d->moduleContext;
    d->moduleContext = 0;

    d->coreCtx->listeners.ModuleChanged(ModuleEvent(ModuleEvent::UNLOADED, this));

    throw;
  }

  d->RemoveModuleResources();
  delete d->moduleContext;
  d->moduleContext = 0;
  d->coreCtx->listeners.ModuleChanged(ModuleEvent(ModuleEvent::UNLOADED, this));
}

ModuleContext* Module::GetModuleContext() const
{
  return d->moduleContext;
}

long Module::GetModuleId() const
{
  return d->info.id;
}

std::string Module::GetLocation() const
{
  return d->info.location;
}

std::string Module::GetName() const
{
  return d->info.name;
}

ModuleVersion Module::GetVersion() const
{
  return d->version;
}

std::string Module::GetProperty(const std::string& key) const
{
  if (d->properties.count(key) == 0) return "";
  return d->properties[key];
}

} // end namespace mitk

std::ostream& operator<<(std::ostream& os, const mitk::Module& module)
{
  os << "Module[" << "id=" << module.GetModuleId() <<
        ", loc=" << module.GetLocation() <<
        ", name=" << module.GetName() << "]";
  return os;
}

std::ostream& operator<<(std::ostream& os, mitk::Module const * module)
{
  return operator<<(os, *module);
}
