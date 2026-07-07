/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "usTestModuleBService.h"

#include <usModuleActivator.h>
#include <mitkLog.h>
#include <usModuleContext.h>
#include <mitkLog.h>


namespace us {

struct TestModuleImportedByB : public TestModuleBService
{

  TestModuleImportedByB(ModuleContext* mc)
  {
    MITK_INFO << "Registering TestModuleImportedByB";
    mc->RegisterService<TestModuleBService>(this);
  }

};

class TestModuleImportedByBActivator : public ModuleActivator
{
public:

  TestModuleImportedByBActivator() : s(nullptr) {}
  ~TestModuleImportedByBActivator() override { delete s; }

  void Load(ModuleContext* context) override
  {
    s = new TestModuleImportedByB(context);
  }

  void Unload(ModuleContext*) override
  {
  }

private:

  TestModuleImportedByB* s;
};

}

US_EXPORT_MODULE_ACTIVATOR(us::TestModuleImportedByBActivator)
