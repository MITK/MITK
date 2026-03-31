/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "usTestModuleBService.h"

#include <usModuleImport.h>
#include <usModuleActivator.h>
#include <mitkLog.h>
#include <usModuleContext.h>
#include <mitkLog.h>

namespace us {

struct TestModuleB : public TestModuleBService
{

  TestModuleB(ModuleContext* mc)
  {
    MITK_INFO << "Registering TestModuleBService";
    mc->RegisterService<TestModuleBService>(this);
  }

};

class TestModuleBActivator : public ModuleActivator
{
public:

  TestModuleBActivator() : s(nullptr) {}
  ~TestModuleBActivator() override { delete s; }

  void Load(ModuleContext* context) override
  {
    s = new TestModuleB(context);
  }

  void Unload(ModuleContext*) override
  {
  }

private:

  TestModuleB* s;
};

}

US_EXPORT_MODULE_ACTIVATOR(us::TestModuleBActivator)

US_IMPORT_MODULE(TestModuleImportedByB)
