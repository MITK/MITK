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

#include "mitkModulePrivate.h"

#include "mitkModule.h"
#include "mitkCoreModuleContext_p.h"
#include "mitkServiceRegistration.h"
#include "mitkServiceReferencePrivate.h"

namespace mitk {

AtomicInt ModulePrivate::idCounter;

ModulePrivate::ModulePrivate(Module* qq, CoreModuleContext* coreCtx,
                             ModuleInfo* info)
  : coreCtx(coreCtx), info(*info), moduleContext(0), moduleActivator(0), q(qq)
{

  std::stringstream propId;
  propId << this->info.id;
  properties[Module::PROP_ID()] = propId.str();

  std::stringstream propModuleDepends;
  std::stringstream propLibDepends;
  std::stringstream propPackageDepends;

  int counter = 0;
  int counter2 = 0;
  std::stringstream ss(this->info.moduleDeps);
  while (ss)
  {
    std::string moduleDep;
    ss >> moduleDep;
    if (!moduleDep.empty())
    {
      Module* dep = ModuleRegistry::GetModule(moduleDep);
      if (dep)
      {
        requiresIds.push_back(dep->GetModuleId());
        if (counter > 0) propModuleDepends << ", ";
        propModuleDepends << moduleDep;
        ++counter;
      }
      else
      {
        requiresLibs.push_back(moduleDep);
        if (counter2 > 0) propLibDepends << ", ";
        propLibDepends << moduleDep;
        ++counter2;
      }
    }
  }

  properties[Module::PROP_MODULE_DEPENDS()] = propModuleDepends.str();
  properties[Module::PROP_LIB_DEPENDS()] = propLibDepends.str();

  counter = 0;
  ss.clear();
  ss.str(this->info.packageDeps);
  while (ss)
  {
    std::string packageDep;
    ss >> packageDep;
    if (!packageDep.empty())
    {
      requiresPackages.push_back(packageDep);
      if (counter > 0) propPackageDepends << ", ";
      propPackageDepends << packageDep;
      ++counter;
    }
  }

  properties[Module::PROP_PACKAGE_DEPENDS()] = propPackageDepends.str();

  if (!this->info.version.empty())
  {
    try
    {
      version = mitk::ModuleVersion(this->info.version);
      properties[Module::PROP_VERSION()] = this->info.version;
    }
    catch (const std::exception& e)
    {
      throw std::invalid_argument(std::string("MITK module does not specify a valid version identifier. Got exception: ") + e.what());
    }
  }

  properties[Module::PROP_LOCATION()] = this->info.location;
  properties[Module::PROP_NAME()] = this->info.name;
  properties[Module::PROP_QT()] = this->info.qtModule ? "true" : "false";
}

ModulePrivate::~ModulePrivate()
{
}

void ModulePrivate::RemoveModuleResources()
{
  coreCtx->listeners.RemoveAllListeners(moduleContext);
 
  std::list<ServiceRegistration> srs;
  coreCtx->services.GetRegisteredByModule(this, srs);
  for (std::list<ServiceRegistration>::iterator i = srs.begin();
       i != srs.end(); ++i)
  {
    try
    {
      i->Unregister();
    }
    catch (const std::logic_error& /*ignore*/)
    {
      // Someone has unregistered the service after stop completed.
      // This should not occur, but we don't want get stuck in
      // an illegal state so we catch it.
    }
  }

  srs.clear();
  coreCtx->services.GetUsedByModule(q, srs);
  for (std::list<ServiceRegistration>::const_iterator i = srs.begin();
       i != srs.end(); ++i)
  {
    i->GetReference().d->UngetService(q, false);
  }
}

}

