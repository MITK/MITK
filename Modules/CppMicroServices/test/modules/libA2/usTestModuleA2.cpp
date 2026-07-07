/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "usTestModuleA2Service.h"

#include <usModuleActivator.h>
#include <mitkLog.h>
#include <usModuleContext.h>
#include <mitkLog.h>

namespace us {

struct TestModuleA2 : public TestModuleA2Service
{

  TestModuleA2(ModuleContext* mc)
  {
    MITK_INFO << "Registering TestModuleA2Service";
    sr = mc->RegisterService<TestModuleA2Service>(this);
  }

  void Unregister()
  {
    if (sr)
    {
      sr.Unregister();
    }
  }

private:

  ServiceRegistration<TestModuleA2Service> sr;
};

class TestModuleA2Activator : public ModuleActivator
{
public:

  TestModuleA2Activator() : s(nullptr) {}

  ~TestModuleA2Activator() override { delete s; }

  void Load(ModuleContext* context) override
  {
    s = new TestModuleA2(context);
  }

  void Unload(ModuleContext* /*context*/) override
  {
    s->Unregister();
  }

private:

  TestModuleA2* s;
};

}

US_EXPORT_MODULE_ACTIVATOR(us::TestModuleA2Activator)
