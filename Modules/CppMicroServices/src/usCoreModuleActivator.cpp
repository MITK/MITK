/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <usModuleActivator.h>

#include <usModule.h>
#include "usModulePrivate.h"
#include "usCoreModuleContext_p.h"

namespace us {

class CoreModuleActivator : public ModuleActivator
{

  void Load(ModuleContext* mc) override
  {
    mc->GetModule()->d->coreCtx->Init();
  }

  void Unload(ModuleContext* /*mc*/) override
  {
    //mc->GetModule()->d->coreCtx->Uninit();
  }

};

}

US_EXPORT_MODULE_ACTIVATOR(us::CoreModuleActivator)
