/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <usModule.h>
#include <usModuleEvent.h>
#include <usModuleFindHook.h>
#include <usModuleEventHook.h>
#include <usModuleContext.h>
#include <usGetModuleContext.h>
#include <usSharedLibrary.h>

#include "usTestingMacros.h"
#include "usTestingConfig.h"

using namespace us;

namespace {

#ifdef US_PLATFORM_WINDOWS
  static const std::string LIB_PATH = US_RUNTIME_OUTPUT_DIRECTORY;
#else
  static const std::string LIB_PATH = US_LIBRARY_OUTPUT_DIRECTORY;
#endif

class TestModuleListener
{
public:

  void ModuleChanged(const ModuleEvent moduleEvent)
  {
    this->events.push_back(moduleEvent);
  }

  std::vector<ModuleEvent> events;
};

class TestModuleFindHook : public ModuleFindHook
{
public:

  void Find(const ModuleContext* /*context*/, ShrinkableVector<Module*>& modules) override
  {
    for (ShrinkableVector<Module*>::iterator i = modules.begin();
         i != modules.end();)
    {
      if ((*i)->GetName() == "TestModuleA")
      {
        i = modules.erase(i);
      }
      else
      {
        ++i;
      }
    }
  }
};

class TestModuleEventHook : public ModuleEventHook
{
public:

  void Event(const ModuleEvent& event, ShrinkableVector<ModuleContext*>& contexts) override
  {
    if (event.GetType() == ModuleEvent::LOADING || event.GetType() == ModuleEvent::UNLOADING)
    {
      contexts.erase(std::remove(contexts.begin(), contexts.end(), GetModuleContext()), contexts.end());
    }
  }
};

void TestFindHook()
{
  SharedLibrary libA(LIB_PATH, "TestModuleA");

  try
  {
    libA.Load();
  }
  catch (const std::exception& e)
  {
    US_TEST_FAILED_MSG(<< "Load module exception: " << e.what())
  }

  Module* moduleA = GetModuleContext()->GetModule("TestModuleA");
  US_TEST_CONDITION_REQUIRED(moduleA != nullptr, "Test for existing module TestModuleA")

  US_TEST_CONDITION(moduleA->GetName() == "TestModuleA", "Test module name")

  US_TEST_CONDITION(moduleA->IsLoaded() == true, "Test if loaded correctly");

  long moduleAId = moduleA->GetModuleId();
  US_TEST_CONDITION_REQUIRED(moduleAId > 0, "Test for valid module id")

  US_TEST_CONDITION_REQUIRED(GetModuleContext()->GetModule(moduleAId) != nullptr, "Test for non-filtered GetModule(long) result")

  TestModuleFindHook findHook;
  ServiceRegistration<ModuleFindHook> findHookReg = GetModuleContext()->RegisterService<ModuleFindHook>(&findHook);

  US_TEST_CONDITION_REQUIRED(GetModuleContext()->GetModule(moduleAId) == nullptr, "Test for filtered GetModule(long) result")

  std::vector<Module*> modules = GetModuleContext()->GetModules();
  for (std::vector<Module*>::iterator i = modules.begin();
       i != modules.end(); ++i)
  {
    if((*i)->GetName() == "TestModuleA")
    {
      US_TEST_FAILED_MSG(<< "TestModuleA not filtered from GetModules()")
    }
  }

  findHookReg.Unregister();

  libA.Unload();
}

void TestEventHook()
{
  TestModuleListener moduleListener;
  GetModuleContext()->AddModuleListener(&moduleListener, &TestModuleListener::ModuleChanged);

  SharedLibrary libA(LIB_PATH, "TestModuleA");
  try
  {
    libA.Load();
  }
  catch (const std::exception& e)
  {
    US_TEST_FAILED_MSG(<< "Load module exception: " << e.what())
  }
  US_TEST_CONDITION_REQUIRED(moduleListener.events.size() == 2, "Test for received load module events")

  libA.Unload();
  US_TEST_CONDITION_REQUIRED(moduleListener.events.size() == 4, "Test for received unload module events")

  TestModuleEventHook eventHook;
  ServiceRegistration<ModuleEventHook> eventHookReg = GetModuleContext()->RegisterService<ModuleEventHook>(&eventHook);

  moduleListener.events.clear();
  try
  {
    libA.Load();
  }
  catch (const std::exception& e)
  {
    US_TEST_FAILED_MSG(<< "Load module exception: " << e.what())
  }

  US_TEST_CONDITION_REQUIRED(moduleListener.events.size() == 1, "Test for filtered load module events")
  US_TEST_CONDITION_REQUIRED(moduleListener.events[0].GetType() == ModuleEvent::LOADED, "Test for LOADED event")

  libA.Unload();
  US_TEST_CONDITION_REQUIRED(moduleListener.events.size() == 2, "Test for filtered unload module events")
  US_TEST_CONDITION_REQUIRED(moduleListener.events[1].GetType() == ModuleEvent::UNLOADED, "Test for UNLOADED event")

  eventHookReg.Unregister();
  GetModuleContext()->RemoveModuleListener(&moduleListener, &TestModuleListener::ModuleChanged);
}

} // end unnamed namespace

int usModuleHooksTest(int /*argc*/, char* /*argv*/[])
{
  US_TEST_BEGIN("ModuleHooksTest");

  TestFindHook();

  TestEventHook();

  US_TEST_END()
}
