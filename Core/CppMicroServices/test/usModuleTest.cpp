/*=============================================================================

  Library: CppMicroServices

  Copyright (c) German Cancer Research Center,
    Division of Medical and Biological Informatics

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

=============================================================================*/

#include <usModule.h>
#include <usModuleEvent.h>
#include <usServiceEvent.h>
#include <usModuleContext.h>
#include <usGetModuleContext.h>
#include <usModuleRegistry.h>
#include <usModuleActivator.h>
#include <usSharedLibrary.h>

#include "usTestUtilModuleListener.h"
#include "usTestingMacros.h"
#include "usTestingConfig.h"

US_USE_NAMESPACE

namespace {

#ifdef US_PLATFORM_WINDOWS
  static const std::string LIB_PATH = US_RUNTIME_OUTPUT_DIRECTORY;
#else
  static const std::string LIB_PATH = US_LIBRARY_OUTPUT_DIRECTORY;
#endif

// Verify that the same member function pointers registered as listeners
// with different receivers works.
void frame02a()
{
  ModuleContext* mc = GetModuleContext();

  TestModuleListener listener1;
  TestModuleListener listener2;

  try
  {
    mc->RemoveModuleListener(&listener1, &TestModuleListener::ModuleChanged);
    mc->AddModuleListener(&listener1, &TestModuleListener::ModuleChanged);
    mc->RemoveModuleListener(&listener2, &TestModuleListener::ModuleChanged);
    mc->AddModuleListener(&listener2, &TestModuleListener::ModuleChanged);
  }
  catch (const std::logic_error& ise)
  {
    US_TEST_FAILED_MSG( << "module listener registration failed " << ise.what()
                        << " : frameSL02a:FAIL" );
  }

  SharedLibrary target(LIB_PATH, "TestModuleA");

#ifdef US_BUILD_SHARED_LIBS
  // Start the test target
  try
  {
    target.Load();
  }
  catch (const std::exception& e)
  {
    US_TEST_FAILED_MSG( << "Failed to load module, got exception: "
                        << e.what() << " + in frameSL02a:FAIL" );
  }

  Module* moduleA = ModuleRegistry::GetModule("TestModuleA Module");
  US_TEST_CONDITION_REQUIRED(moduleA != 0, "Test for existing module TestModuleA")
#endif

  std::vector<ModuleEvent> pEvts;
#ifdef US_BUILD_SHARED_LIBS
  pEvts.push_back(ModuleEvent(ModuleEvent::LOADING, moduleA));
  pEvts.push_back(ModuleEvent(ModuleEvent::LOADED, moduleA));
#endif

  std::vector<ServiceEvent> seEvts;

  US_TEST_CONDITION(listener1.CheckListenerEvents(pEvts, seEvts), "Check first module listener")
  US_TEST_CONDITION(listener2.CheckListenerEvents(pEvts, seEvts), "Check second module listener")

  mc->RemoveModuleListener(&listener1, &TestModuleListener::ModuleChanged);
  mc->RemoveModuleListener(&listener2, &TestModuleListener::ModuleChanged);

  target.Unload();
}

// Verify information from the ModuleInfo struct
void frame005a(ModuleContext* mc)
{
  Module* m = mc->GetModule();

  // check expected headers

#ifdef US_BUILD_SHARED_LIBS
  US_TEST_CONDITION("CppMicroServicesTestDriver" == m->GetName(), "Test module name");
  US_TEST_CONDITION(ModuleVersion(0,1,0) == m->GetVersion(), "Test module version")
#else
  US_TEST_CONDITION("CppMicroServices" == m->GetName(), "Test module name");
  US_TEST_CONDITION(ModuleVersion(1,99,0) == m->GetVersion(), "Test module version")
#endif
}

// Get context id, location and status of the module
void frame010a(ModuleContext* mc)
{
  Module* m = mc->GetModule();

  long int contextid = m->GetModuleId();
  US_DEBUG << "CONTEXT ID:" << contextid;

  std::string location = m->GetLocation();
  US_DEBUG << "LOCATION:" << location;
  US_TEST_CONDITION(!location.empty(), "Test for non-empty module location")

  US_TEST_CONDITION(m->IsLoaded(), "Test for loaded flag")
}

//----------------------------------------------------------------------------
//Test result of GetService(ServiceReference()). Should throw std::invalid_argument
void frame018a(ModuleContext* mc)
{
  try
  {
    mc->GetService(ServiceReferenceU());
    US_DEBUG << "Got service object, expected std::invalid_argument exception";
    US_TEST_FAILED_MSG(<< "Got service object, excpected std::invalid_argument exception")
  }
  catch (const std::invalid_argument& )
  {}
  catch (...)
  {
    US_TEST_FAILED_MSG(<< "Got wrong exception, expected std::invalid_argument")
  }
}

// Load libA and check that it exists and that the service it registers exists,
// also check that the expected events occur
void frame020a(ModuleContext* mc, TestModuleListener& listener,
#ifdef US_BUILD_SHARED_LIBS
               SharedLibrary& libA)
{
  try
  {
    libA.Load();
  }
  catch (const std::exception& e)
  {
    US_TEST_FAILED_MSG(<< "Load module exception: " << e.what())
  }

  Module* moduleA = ModuleRegistry::GetModule("TestModuleA Module");
  US_TEST_CONDITION_REQUIRED(moduleA != 0, "Test for existing module TestModuleA")

  US_TEST_CONDITION(moduleA->GetName() == "TestModuleA Module", "Test module name")
#else
               SharedLibrary& /*libA*/)
{
#endif

  // Check if libA registered the expected service
  try
  {
    ServiceReferenceU sr1 = mc->GetServiceReference("org.cppmicroservices.TestModuleAService");
    InterfaceMap o1 = mc->GetService(sr1);
    US_TEST_CONDITION(!o1.empty(), "Test if service object found");

    try
    {
      US_TEST_CONDITION(mc->UngetService(sr1), "Test if Service UnGet returns true");
    }
    catch (const std::logic_error le)
    {
      US_TEST_FAILED_MSG(<< "UnGetService exception: " << le.what())
    }

    // check the listeners for events
    std::vector<ModuleEvent> pEvts;
#ifdef US_BUILD_SHARED_LIBS
    pEvts.push_back(ModuleEvent(ModuleEvent::LOADING, moduleA));
    pEvts.push_back(ModuleEvent(ModuleEvent::LOADED, moduleA));
#endif

    std::vector<ServiceEvent> seEvts;
#ifdef US_BUILD_SHARED_LIBS
    seEvts.push_back(ServiceEvent(ServiceEvent::REGISTERED, sr1));
#endif

    US_TEST_CONDITION(listener.CheckListenerEvents(pEvts, seEvts), "Test for unexpected events");

  }
  catch (const ServiceException& /*se*/)
  {
    US_TEST_FAILED_MSG(<< "test module, expected service not found");
  }

#ifdef US_BUILD_SHARED_LIBS
  US_TEST_CONDITION(moduleA->IsLoaded() == true, "Test if loaded correctly");
#endif
}


// Unload libA and check for correct events
void frame030b(ModuleContext* mc, TestModuleListener& listener, SharedLibrary& libA)
{
#ifdef US_BUILD_SHARED_LIBS
  Module* moduleA = ModuleRegistry::GetModule("TestModuleA Module");
  US_TEST_CONDITION_REQUIRED(moduleA != 0, "Test for non-null module")
#endif

  ServiceReferenceU sr1
      = mc->GetServiceReference("org.cppmicroservices.TestModuleAService");
  US_TEST_CONDITION(sr1, "Test for valid service reference")

  try
  {
    libA.Unload();
#ifdef US_BUILD_SHARED_LIBS
    US_TEST_CONDITION(moduleA->IsLoaded() == false, "Test for unloaded state")
#endif
  }
  catch (const std::exception& e)
  {
    US_TEST_FAILED_MSG(<< "UnLoad module exception: " << e.what())
  }

  std::vector<ModuleEvent> pEvts;
#ifdef US_BUILD_SHARED_LIBS
  pEvts.push_back(ModuleEvent(ModuleEvent::UNLOADING, moduleA));
  pEvts.push_back(ModuleEvent(ModuleEvent::UNLOADED, moduleA));
#endif

  std::vector<ServiceEvent> seEvts;
#ifdef US_BUILD_SHARED_LIBS
  seEvts.push_back(ServiceEvent(ServiceEvent::UNREGISTERING, sr1));
#endif

  US_TEST_CONDITION(listener.CheckListenerEvents(pEvts, seEvts), "Test for unexpected events");
}


struct LocalListener {
  void ServiceChanged(const ServiceEvent) {}
};

// Add a service listener with a broken LDAP filter to Get an exception
void frame045a(ModuleContext* mc)
{
  LocalListener sListen1;
  std::string brokenFilter = "A broken LDAP filter";

  try
  {
    mc->AddServiceListener(&sListen1, &LocalListener::ServiceChanged, brokenFilter);
  }
  catch (const std::invalid_argument& /*ia*/)
  {
    //assertEquals("InvalidSyntaxException.GetFilter should be same as input string", brokenFilter, ise.GetFilter());
  }
  catch (...)
  {
    US_TEST_FAILED_MSG(<< "test module, wrong exception on broken LDAP filter:");
  }
}

} // end unnamed namespace

int usModuleTest(int /*argc*/, char* /*argv*/[])
{
  US_TEST_BEGIN("ModuleTest");

  frame02a();

  ModuleContext* mc = GetModuleContext();
  TestModuleListener listener;

  try
  {
    mc->AddModuleListener(&listener, &TestModuleListener::ModuleChanged);
  }
  catch (const std::logic_error& ise)
  {
    US_TEST_OUTPUT( << "module listener registration failed " << ise.what() );
    throw;
  }

  try
  {
    mc->AddServiceListener(&listener, &TestModuleListener::ServiceChanged);
  }
  catch (const std::logic_error& ise)
  {
    US_TEST_OUTPUT( << "service listener registration failed " << ise.what() );
    throw;
  }

  frame005a(mc);
  frame010a(mc);
  frame018a(mc);

  SharedLibrary libA(LIB_PATH, "TestModuleA");
  frame020a(mc, listener, libA);
  frame030b(mc, listener, libA);

  frame045a(mc);

  mc->RemoveModuleListener(&listener, &TestModuleListener::ModuleChanged);
  mc->RemoveServiceListener(&listener, &TestModuleListener::ServiceChanged);

  US_TEST_END()
}
