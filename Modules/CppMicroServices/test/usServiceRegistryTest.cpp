/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <usLDAPFilter.h>

#include "usTestingMacros.h"
#include <usServiceInterface.h>
#include <usGetModuleContext.h>
#include <usModule.h>
#include <usModuleContext.h>

#include <stdexcept>
#include <vector>

using namespace us;

struct ITestServiceA
{
  virtual ~ITestServiceA() {}
};


void TestServiceInterfaceId()
{
  US_TEST_CONDITION(us_service_interface_iid<int>() == "int", "Service interface id int")
  US_TEST_CONDITION(us_service_interface_iid<ITestServiceA>() == "ITestServiceA", "Service interface id ITestServiceA")
}

void TestMultipleServiceRegistrations()
{
  struct TestServiceA : public ITestServiceA
  {
  };

  ModuleContext* context = GetModuleContext();

  TestServiceA s1;
  TestServiceA s2;

  ServiceRegistration<ITestServiceA> reg1 = context->RegisterService<ITestServiceA>(&s1);
  ServiceRegistration<ITestServiceA> reg2 = context->RegisterService<ITestServiceA>(&s2);

  std::vector<ServiceReference<ITestServiceA> > refs = context->GetServiceReferences<ITestServiceA>();
  US_TEST_CONDITION_REQUIRED(refs.size() == 2, "Testing for two registered ITestServiceA services")

  reg2.Unregister();
  refs = context->GetServiceReferences<ITestServiceA>();
  US_TEST_CONDITION_REQUIRED(refs.size() == 1, "Testing for one registered ITestServiceA services")

  reg1.Unregister();
  refs = context->GetServiceReferences<ITestServiceA>();
  US_TEST_CONDITION_REQUIRED(refs.empty(), "Testing for no ITestServiceA services")

  ServiceReference<ITestServiceA> ref = context->GetServiceReference<ITestServiceA>();
  US_TEST_CONDITION_REQUIRED(!ref, "Testing for invalid service reference")
}

void TestServicePropertiesUpdate()
{
  struct TestServiceA : public ITestServiceA
  {
  };

  ModuleContext* context = GetModuleContext();

  TestServiceA s1;
  ServiceProperties props;
  props["string"] = std::string("A std::string");
  props["bool"] = false;
  const char* str = "A const char*";
  props["const char*"] = str;

  ServiceRegistration<ITestServiceA> reg1 = context->RegisterService<ITestServiceA>(&s1, props);
  ServiceReference<ITestServiceA> ref1 = context->GetServiceReference<ITestServiceA>();

  US_TEST_CONDITION_REQUIRED(context->GetServiceReferences<ITestServiceA>().size() == 1, "Testing service count")
  US_TEST_CONDITION_REQUIRED(any_cast<bool>(ref1.GetProperty("bool")) == false, "Testing bool property")

  // register second service with higher rank
  TestServiceA s2;
  ServiceProperties props2;
  props2[ServiceConstants::SERVICE_RANKING()] = 50;

  ServiceRegistration<ITestServiceA> reg2 = context->RegisterService<ITestServiceA>(&s2, props2);

  // Get the service with the highest rank, this should be s2.
  ServiceReference<ITestServiceA> ref2 = context->GetServiceReference<ITestServiceA>();
  TestServiceA* service = dynamic_cast<TestServiceA*>(context->GetService(ref2));
  US_TEST_CONDITION_REQUIRED(service == &s2, "Testing highest service rank")

  props["bool"] = true;
  // change the service ranking
  props[ServiceConstants::SERVICE_RANKING()] = 100;
  reg1.SetProperties(props);

  US_TEST_CONDITION_REQUIRED(context->GetServiceReferences<ITestServiceA>().size() == 2, "Testing service count")
  US_TEST_CONDITION_REQUIRED(any_cast<bool>(ref1.GetProperty("bool")) == true, "Testing bool property")
  US_TEST_CONDITION_REQUIRED(any_cast<int>(ref1.GetProperty(ServiceConstants::SERVICE_RANKING())) == 100, "Testing updated ranking")

  // Service with the highest ranking should now be s1
  service = dynamic_cast<TestServiceA*>(context->GetService<ITestServiceA>(ref1));
  US_TEST_CONDITION_REQUIRED(service == &s1, "Testing highest service rank")

  reg1.Unregister();
  US_TEST_CONDITION_REQUIRED(context->GetServiceReferences<ITestServiceA>("").size() == 1, "Testing service count")

  service = dynamic_cast<TestServiceA*>(context->GetService<ITestServiceA>(ref2));
  US_TEST_CONDITION_REQUIRED(service == &s2, "Testing highest service rank")

  reg2.Unregister();
  US_TEST_CONDITION_REQUIRED(context->GetServiceReferences<ITestServiceA>().empty(), "Testing service count")
}


void TestInvalidServiceReference()
{
  // Regression: GetProperty/GetPropertyKeys/GetUsingModules on a default-
  // constructed (invalid) ServiceReferenceBase must not crash.
  ServiceReferenceU invalidRef;
  US_TEST_CONDITION_REQUIRED(!invalidRef, "Default-constructed reference is invalid")
  US_TEST_CONDITION(invalidRef.GetProperty("anything").Empty(),
                    "GetProperty on invalid ref returns empty Any")

  std::vector<std::string> keys;
  keys.push_back("sentinel");
  invalidRef.GetPropertyKeys(keys);
  US_TEST_CONDITION(keys.size() == 1 && keys[0] == "sentinel",
                    "GetPropertyKeys on invalid ref does not touch output")

  std::vector<Module*> modules;
  modules.push_back(nullptr);
  invalidRef.GetUsingModules(modules);
  US_TEST_CONDITION(modules.size() == 1 && modules[0] == nullptr,
                    "GetUsingModules on invalid ref does not touch output")
}

void TestPropertyKeyPrefixCollision()
{
  // Regression: ServicePropertiesImpl::Find compared only key.size()
  // characters, so a lookup for "mykey_long" would falsely match a stored
  // "mykey" (prefix collision).
  struct TestServiceA : public ITestServiceA
  {
  };

  ModuleContext* context = GetModuleContext();

  TestServiceA s;
  ServiceProperties props;
  props["mykey"] = std::string("short");
  ServiceRegistration<ITestServiceA> reg = context->RegisterService<ITestServiceA>(&s, props);
  ServiceReference<ITestServiceA> ref = reg.GetReference();

  US_TEST_CONDITION(any_cast<std::string>(ref.GetProperty("mykey")) == "short",
                    "Stored key resolves to its value")
  US_TEST_CONDITION(ref.GetProperty("mykey_long").Empty(),
                    "Longer key with stored key as prefix must not resolve")
  US_TEST_CONDITION(ref.GetProperty("myk").Empty(),
                    "Shorter key that is a prefix of a stored key must not resolve")

  reg.Unregister();
}

int usServiceRegistryTest(int /*argc*/, char* /*argv*/[])
{
  US_TEST_BEGIN("ServiceRegistryTest");

  TestServiceInterfaceId();
  TestMultipleServiceRegistrations();
  TestServicePropertiesUpdate();
  TestInvalidServiceReference();
  TestPropertyKeyPrefixCollision();

  US_TEST_END()
}
