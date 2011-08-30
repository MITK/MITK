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

#include "mitkModuleContext.h"

#include "mitkModuleRegistry.h"
#include "mitkModulePrivate.h"
#include "mitkCoreModuleContext_p.h"
#include "mitkServiceRegistry_p.h"
#include "mitkServiceReferencePrivate.h"

namespace mitk {

class ModuleContextPrivate {

public:

  ModuleContextPrivate(ModulePrivate* module)
  : module(module)
  {}

  ModulePrivate* module;
};


ModuleContext::ModuleContext(ModulePrivate* module)
  : d(new ModuleContextPrivate(module))
{}

ModuleContext::~ModuleContext()
{
  delete d;
}

Module* ModuleContext::GetModule() const
{
  return d->module->q;
}

Module* ModuleContext::GetModule(long id) const
{
  return ModuleRegistry::GetModule(id);
}

void ModuleContext::GetModules(std::vector<Module*>& modules) const
{
  ModuleRegistry::GetModules(modules);
}

ServiceRegistration ModuleContext::RegisterService(const std::list<std::string>& clazzes,
                                                   itk::LightObject* service,
                                                   const ServiceProperties& properties)
{
  return d->module->coreCtx->services.RegisterService(d->module, clazzes, service, properties);
}

ServiceRegistration ModuleContext::RegisterService(const char* clazz, itk::LightObject* service,
                                                   const ServiceProperties& properties)
{
  std::list<std::string> clazzes;
  clazzes.push_back(clazz);
  return d->module->coreCtx->services.RegisterService(d->module, clazzes, service, properties);
}

std::list<ServiceReference> ModuleContext::GetServiceReferences(const std::string& clazz,
                                                                const std::string& filter)
{
  std::list<ServiceReference> result;
  d->module->coreCtx->services.Get(clazz, filter, 0, result);
  return result;
}

ServiceReference ModuleContext::GetServiceReference(const std::string& clazz)
{
  return d->module->coreCtx->services.Get(d->module, clazz);
}

itk::LightObject* ModuleContext::GetService(const ServiceReference& reference)
{
  if (!reference)
  {
    throw std::invalid_argument("Default constructed mitk::ServiceReference is not a valid input to getService()");
  }
  ServiceReference internalRef(reference);
  return internalRef.d->GetService(d->module->q);
}

bool ModuleContext::UngetService(const ServiceReference& reference)
{
  ServiceReference ref = reference;
  return ref.d->UngetService(d->module->q, true);
}

void ModuleContext::AddServiceListener(const ServiceListenerDelegate& delegate,
                                       const std::string& filter)
{
  d->module->coreCtx->listeners.AddServiceListener(this, delegate, filter);
}

void ModuleContext::RemoveServiceListener(const ServiceListenerDelegate& delegate)
{
  d->module->coreCtx->listeners.RemoveServiceListener(this, delegate);
}

void ModuleContext::AddModuleListener(const ModuleListenerDelegate& delegate)
{
  d->module->coreCtx->listeners.AddModuleListener(this, delegate);
}

void ModuleContext::RemoveModuleListener(const ModuleListenerDelegate& delegate)
{
  d->module->coreCtx->listeners.RemoveModuleListener(this, delegate);
}

}

