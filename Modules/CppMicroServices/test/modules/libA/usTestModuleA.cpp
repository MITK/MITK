/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "usTestModuleAService.h"

#include <usModuleActivator.h>
#include <mitkLog.h>
#include <usModuleContext.h>
#include <mitkLog.h>
#include <usGlobalConfig.h>
#include <mitkLog.h>

namespace us {

struct TestModuleA : public TestModuleAService
{

  TestModuleA(ModuleContext* mc)
  {
    MITK_INFO << "Registering TestModuleAService";
    sr = mc->RegisterService<TestModuleAService>(this);
  }

  void Unregister()
  {
    if (sr)
    {
      sr.Unregister();
      sr = 0;
    }
  }

private:

  ServiceRegistration<TestModuleAService> sr;

};

class TestModuleAActivator : public ModuleActivator
{
public:

  TestModuleAActivator() : s(nullptr) {}
  ~TestModuleAActivator() override { delete s; }

  void Load(ModuleContext* context) override
  {
    s = new TestModuleA(context);
  }

  void Unload(ModuleContext*) override
  {
  }

private:

  TestModuleA* s;
};

}

US_EXPORT_MODULE_ACTIVATOR(us::TestModuleAActivator)
