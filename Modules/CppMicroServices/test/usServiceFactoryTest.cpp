/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <usGetModuleContext.h>
#include <usModuleContext.h>
#include <usModule.h>
#include <usModuleRegistry.h>
#include <usServiceObjects.h>
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

} // end unnamed namespace


namespace us {

struct TestModuleH
{
  virtual ~TestModuleH() {}
};

struct TestModuleH2
{
  virtual ~TestModuleH2() {}
};

}


void TestServiceFactoryModuleScope()
{

  // Install and start test module H, a service factory and test that the methods
  // in that interface works.

  SharedLibrary target(LIB_PATH, "TestModuleH");

  try
  {
    target.Load();
  }
  catch (const std::exception& e)
  {
    US_TEST_FAILED_MSG( << "Failed to load module, got exception: " << e.what())
  }

  Module* moduleH = ModuleRegistry::GetModule("TestModuleH");
  US_TEST_CONDITION_REQUIRED(moduleH != nullptr, "Test for existing module TestModuleH")

  std::vector<ServiceReferenceU> registeredRefs = moduleH->GetRegisteredServices();
  US_TEST_CONDITION_REQUIRED(registeredRefs.size() == 2, "# of registered services")
  US_TEST_CONDITION(registeredRefs[0].GetProperty(ServiceConstants::SERVICE_SCOPE()).ToString() == ServiceConstants::SCOPE_MODULE(), "service scope")
  US_TEST_CONDITION(registeredRefs[1].GetProperty(ServiceConstants::SERVICE_SCOPE()).ToString() == ServiceConstants::SCOPE_PROTOTYPE(), "service scope")

  ModuleContext* mc = GetModuleContext();
  // Check that a service reference exist
  const ServiceReferenceU sr1 = mc->GetServiceReference("us::TestModuleH");
  US_TEST_CONDITION_REQUIRED(sr1, "Service shall be present.")
  US_TEST_CONDITION(sr1.GetProperty(ServiceConstants::SERVICE_SCOPE()).ToString() == ServiceConstants::SCOPE_MODULE(), "service scope")

  InterfaceMap service = mc->GetService(sr1);
  US_TEST_CONDITION_REQUIRED(service.size() >= 1, "GetService()")
  InterfaceMap::const_iterator serviceIter = service.find("us::TestModuleH");
  US_TEST_CONDITION_REQUIRED(serviceIter != service.end(), "GetService()")
  US_TEST_CONDITION_REQUIRED(serviceIter->second != nullptr, "GetService()")

  InterfaceMap service2 = mc->GetService(sr1);
  US_TEST_CONDITION(service == service2, "Same service pointer")

  std::vector<ServiceReferenceU> usedRefs = mc->GetModule()->GetServicesInUse();
  US_TEST_CONDITION_REQUIRED(usedRefs.size() == 1, "services in use")
  US_TEST_CONDITION(usedRefs[0] == sr1, "service ref in use")

  InterfaceMap service3 = moduleH->GetModuleContext()->GetService(sr1);
  US_TEST_CONDITION(service != service3, "Different service pointer")
  US_TEST_CONDITION(moduleH->GetModuleContext()->UngetService(sr1), "UngetService()")

  US_TEST_CONDITION_REQUIRED(mc->UngetService(sr1) == false, "ungetService()")
  US_TEST_CONDITION_REQUIRED(mc->UngetService(sr1) == true, "ungetService()")

  target.Unload();
}

void TestServiceFactoryPrototypeScope()
{

  // Install and start test module H, a service factory and test that the methods
  // in that interface works.

  SharedLibrary target(LIB_PATH, "TestModuleH");

  try
  {
    target.Load();
  }
  catch (const std::exception& e)
  {
    US_TEST_FAILED_MSG( << "Failed to load module, got exception: " << e.what())
  }

  Module* moduleH = ModuleRegistry::GetModule("TestModuleH");
  US_TEST_CONDITION_REQUIRED(moduleH != nullptr, "Test for existing module TestModuleH")

  ModuleContext* mc = GetModuleContext();
  // Check that a service reference exist
  const ServiceReference<TestModuleH2> sr1 = mc->GetServiceReference<TestModuleH2>();
  US_TEST_CONDITION_REQUIRED(sr1, "Service shall be present.")
  US_TEST_CONDITION(sr1.GetProperty(ServiceConstants::SERVICE_SCOPE()).ToString() == ServiceConstants::SCOPE_PROTOTYPE(), "service scope")

  ServiceObjects<TestModuleH2> svcObjects = mc->GetServiceObjects(sr1);
  TestModuleH2* prototypeServiceH2 = svcObjects.GetService();

  const ServiceReferenceU sr1void = mc->GetServiceReference(us_service_interface_iid<TestModuleH2>());
  ServiceObjects<void> svcObjectsVoid = mc->GetServiceObjects(sr1void);
  InterfaceMap prototypeServiceH2Void = svcObjectsVoid.GetService();
  US_TEST_CONDITION_REQUIRED(prototypeServiceH2Void.find(us_service_interface_iid<TestModuleH2>()) != prototypeServiceH2Void.end(),
                             "ServiceObjects<void>::GetService()")

  // There should be only one service in use
  US_TEST_CONDITION_REQUIRED(mc->GetModule()->GetServicesInUse().size() == 1, "services in use")

  TestModuleH2* moduleScopeService = mc->GetService(sr1);
  US_TEST_CONDITION_REQUIRED(moduleScopeService && moduleScopeService != prototypeServiceH2, "GetService()")

  US_TEST_CONDITION_REQUIRED(prototypeServiceH2 != prototypeServiceH2Void.find(us_service_interface_iid<TestModuleH2>())->second,
                             "GetService()")

  svcObjectsVoid.UngetService(prototypeServiceH2Void);

  TestModuleH2* moduleScopeService2 = mc->GetService(sr1);
  US_TEST_CONDITION(moduleScopeService == moduleScopeService2, "Same service pointer")

  std::vector<ServiceReferenceU> usedRefs = mc->GetModule()->GetServicesInUse();
  US_TEST_CONDITION_REQUIRED(usedRefs.size() == 1, "services in use")
  US_TEST_CONDITION(usedRefs[0] == sr1, "service ref in use")

  std::string filter = "(" + ServiceConstants::SERVICE_ID() + "=" + sr1.GetProperty(ServiceConstants::SERVICE_ID()).ToString() + ")";
  const ServiceReference<TestModuleH> sr2 = mc->GetServiceReferences<TestModuleH>(filter).front();
  US_TEST_CONDITION_REQUIRED(sr2, "Service shall be present.")
  US_TEST_CONDITION(sr2.GetProperty(ServiceConstants::SERVICE_SCOPE()).ToString() == ServiceConstants::SCOPE_PROTOTYPE(), "service scope")
  US_TEST_CONDITION(any_cast<long>(sr2.GetProperty(ServiceConstants::SERVICE_ID())) == any_cast<long>(sr1.GetProperty(ServiceConstants::SERVICE_ID())), "same service id")

  try
  {
    svcObjects.UngetService(moduleScopeService2);
    US_TEST_FAILED_MSG(<< "std::invalid_argument exception expected")
  }
  catch (const std::invalid_argument&)
  {
    // this is expected
  }

  // There should still be only one service in use
  usedRefs = mc->GetModule()->GetServicesInUse();
  US_TEST_CONDITION_REQUIRED(usedRefs.size() == 1, "services in use")

  ServiceObjects<TestModuleH2> svcObjects2 = svcObjects;
  ServiceObjects<TestModuleH2> svcObjects3 = mc->GetServiceObjects(sr1);
  try
  {
    svcObjects3.UngetService(prototypeServiceH2);
    US_TEST_FAILED_MSG(<< "std::invalid_argument exception expected")
  }
  catch (const std::invalid_argument&)
  {
    // this is expected
  }

  svcObjects2.UngetService(prototypeServiceH2);
  prototypeServiceH2 = svcObjects2.GetService();
  TestModuleH2* prototypeServiceH2_2 = svcObjects3.GetService();

  US_TEST_CONDITION_REQUIRED(prototypeServiceH2_2 && prototypeServiceH2_2 != prototypeServiceH2, "prototype service")

  svcObjects2.UngetService(prototypeServiceH2);
  svcObjects3.UngetService(prototypeServiceH2_2);
  US_TEST_CONDITION_REQUIRED(mc->UngetService(sr1) == false, "ungetService()")
  US_TEST_CONDITION_REQUIRED(mc->UngetService(sr1) == true, "ungetService()")

  target.Unload();
}

int usServiceFactoryTest(int /*argc*/, char* /*argv*/[])
{
  US_TEST_BEGIN("ServiceFactoryTest");

  TestServiceFactoryModuleScope();
  TestServiceFactoryPrototypeScope();

  US_TEST_END()
}
