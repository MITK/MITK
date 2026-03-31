/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <usModuleContext.h>
#include <usGetModuleContext.h>
#include <usModuleRegistry.h>
#include <usModule.h>
#include <usModuleResource.h>
#include <usModuleResourceStream.h>
#include <usSharedLibrary.h>

#include "usTestingMacros.h"
#include "usTestingConfig.h"

#include <cassert>

#include <set>

using namespace us;

namespace {

std::string GetResourceContent(const ModuleResource& resource)
{
  std::string line;
  ModuleResourceStream rs(resource);
  std::getline(rs, line);
  return line;
}

struct ResourceComparator {
  bool operator()(const ModuleResource& mr1, const ModuleResource& mr2) const
  {
    return mr1 < mr2;
  }
};

void testResourceOperators(Module* module)
{
  std::vector<ModuleResource> resources = module->FindResources("", "res.txt", false);
  US_TEST_CONDITION_REQUIRED(resources.size() == 1, "Check resource count")
}

void testResourcesWithStaticImport(Module* module)
{
  ModuleResource resource = module->GetResource("res.txt");
  US_TEST_CONDITION_REQUIRED(resource.IsValid(), "Check valid res.txt resource")
  std::string line = GetResourceContent(resource);
  US_TEST_CONDITION(line == "dynamic resource", "Check dynamic resource content")

  resource = module->GetResource("dynamic.txt");
  US_TEST_CONDITION_REQUIRED(resource.IsValid(), "Check valid dynamic.txt resource")
  line = GetResourceContent(resource);
  US_TEST_CONDITION(line == "dynamic", "Check dynamic resource content")

  resource = module->GetResource("static.txt");
  US_TEST_CONDITION_REQUIRED(!resource.IsValid(), "Check in-valid static.txt resource")

  Module* importedByBModule = ModuleRegistry::GetModule("TestModuleImportedByB");
  US_TEST_CONDITION_REQUIRED(importedByBModule != nullptr, "Check valid static module")
  resource = importedByBModule->GetResource("static.txt");
  US_TEST_CONDITION_REQUIRED(resource.IsValid(), "Check valid static.txt resource")
  line = GetResourceContent(resource);
  US_TEST_CONDITION(line == "static", "Check static resource content")

  std::vector<ModuleResource> resources = module->FindResources("", "*.txt", false);
  std::stable_sort(resources.begin(), resources.end(), ResourceComparator());
  std::vector<ModuleResource> importedResources = importedByBModule->FindResources("", "*.txt", false);
  std::stable_sort(importedResources.begin(), importedResources.end(), ResourceComparator());

  US_TEST_CONDITION(resources.size() == 2, "Check resource count")
  US_TEST_CONDITION(importedResources.size() == 2, "Check resource count")
  line = GetResourceContent(resources[0]);
  US_TEST_CONDITION(line == "dynamic", "Check dynamic.txt resource content")
  line = GetResourceContent(resources[1]);
  US_TEST_CONDITION(line == "dynamic resource", "Check res.txt (from importing module) resource content")
  line = GetResourceContent(importedResources[0]);
  US_TEST_CONDITION(line == "static resource", "Check res.txt (from imported module) resource content")
  line = GetResourceContent(importedResources[1]);
  US_TEST_CONDITION(line == "static", "Check static.txt (from importing module) resource content")
}

} // end unnamed namespace


int usStaticModuleResourceTest(int /*argc*/, char* /*argv*/[])
{
  US_TEST_BEGIN("StaticModuleResourceTest");

  assert(GetModuleContext());

#ifdef US_PLATFORM_WINDOWS
  const std::string LIB_PATH = US_RUNTIME_OUTPUT_DIRECTORY;
#else
  const std::string LIB_PATH = US_LIBRARY_OUTPUT_DIRECTORY;
#endif

  SharedLibrary libB(LIB_PATH, "TestModuleB");

  try
  {
    libB.Load();
  }
  catch (const std::exception& e)
  {
    US_TEST_FAILED_MSG(<< "Load module exception: " << e.what())
  }

  Module* module = ModuleRegistry::GetModule("TestModuleB");
  US_TEST_CONDITION_REQUIRED(module != nullptr, "Test for existing module TestModuleB")
  US_TEST_CONDITION(module->GetName() == "TestModuleB", "Test module name")

  testResourceOperators(module);
  testResourcesWithStaticImport(module);

  ModuleResource resource = ModuleRegistry::GetModule("TestModuleImportedByB")->GetResource("static.txt");
  US_TEST_CONDITION_REQUIRED(resource.IsValid(), "Check valid static.txt resource")
  libB.Unload();
  US_TEST_CONDITION_REQUIRED(resource.IsValid() == true, "Check still valid static.txt resource")

  US_TEST_END()
}
