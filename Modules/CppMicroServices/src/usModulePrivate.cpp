/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "usModulePrivate.h"

#include <usModule.h>
#include <usModuleContext.h>
#include <usModuleActivator.h>
#include <usModuleUtils_p.h>
#include <usModuleSettings.h>
#include <usModuleResource.h>
#include <usModuleResourceStream.h>
#include "usCoreModuleContext_p.h"
#include <usServiceRegistration.h>
#include "usServiceReferenceBasePrivate.h"

#include <algorithm>
#include <iterator>
#include <cassert>
#include <cstring>

namespace us {

AtomicInt ModulePrivate::idCounter;

ModulePrivate::ModulePrivate(Module* qq, CoreModuleContext* coreCtx,
                             ModuleInfo* info)
  : coreCtx(coreCtx)
  , info(*info)
  , resourceContainer(info)
  , moduleContext(nullptr)
  , moduleActivator(nullptr)
  , q(qq)
{
  // Check if the module provides a manifest.json file and if yes, parse it.
  if (resourceContainer.IsValid())
  {
    ModuleResource manifestRes("/manifest.json", resourceContainer);
    if (manifestRes)
    {
      ModuleResourceStream manifestStream(manifestRes);
      try
      {
        moduleManifest.Parse(manifestStream);
      }
      catch (const std::exception& e)
      {
        MITK_ERROR << "Parsing of manifest.json for module " << info->location << " failed: " << e.what();
      }
    }
  }

  // Check if we got version information and validate the version identifier
  if (moduleManifest.Contains(Module::PROP_VERSION()))
  {
    Any versionAny = moduleManifest.GetValue(Module::PROP_VERSION());
    std::string errMsg;
    if (versionAny.Type() != typeid(std::string))
    {
      errMsg = std::string("The version identifier must be a string");
    }
    try
    {
      version = ModuleVersion(versionAny.ToString());
    }
    catch (const std::exception& e)
    {
      errMsg = std::string("The version identifier is invalid: ") + e.what();
    }

    if (!errMsg.empty())
    {
      throw std::invalid_argument(std::string("The Json value for ") + Module::PROP_VERSION() + " for module " +
                                  info->location + " is not valid: " + errMsg);
    }
  }

  std::stringstream propId;
  propId << this->info.id;
  moduleManifest.SetValue(Module::PROP_ID(), propId.str());
  moduleManifest.SetValue(Module::PROP_LOCATION(), this->info.location);
  moduleManifest.SetValue(Module::PROP_NAME(), this->info.name);

  if (moduleManifest.Contains(Module::PROP_AUTOLOAD_DIR()))
  {
    this->info.autoLoadDir = moduleManifest.GetValue(Module::PROP_AUTOLOAD_DIR()).ToString();
  }
  else
  {
    this->info.autoLoadDir = this->info.name;
    moduleManifest.SetValue(Module::PROP_AUTOLOAD_DIR(), Any(this->info.autoLoadDir));
  }
}

ModulePrivate::~ModulePrivate()
{
  delete moduleContext;
}

void ModulePrivate::RemoveModuleResources()
{
  coreCtx->listeners.RemoveAllListeners(moduleContext);

  std::vector<ServiceRegistrationBase> srs;
  coreCtx->services.GetRegisteredByModule(this, srs);
  for (std::vector<ServiceRegistrationBase>::iterator i = srs.begin();
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
  for (std::vector<ServiceRegistrationBase>::const_iterator i = srs.begin();
       i != srs.end(); ++i)
  {
    i->GetReference(std::string()).d->UngetService(q, false);
  }
}

}
