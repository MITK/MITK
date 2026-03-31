/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#include <usModuleActivator.h>
#include <mitkLog.h>
#include <usModuleContext.h>
#include <mitkLog.h>
#include <usServiceRegistration.h>

#include "usFooService.h"

namespace us {

class ActivatorSL4 :
  public ModuleActivator, public FooService
{

public:

  ~ActivatorSL4() override
  {

  }

  void foo() override
  {
    MITK_INFO << "TestModuleSL4: Doing foo";
  }

  void Load(ModuleContext* context) override
  {
    sr = context->RegisterService<FooService>(this);
    MITK_INFO << "TestModuleSL4: Registered " << sr;
  }

  void Unload(ModuleContext* /*context*/) override
  {
  }

private:

  ServiceRegistration<FooService> sr;
};

}

US_EXPORT_MODULE_ACTIVATOR(us::ActivatorSL4)
