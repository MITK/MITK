/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#include <usServiceInterface.h>
#include <usModuleActivator.h>
#include <usServiceFactory.h>
#include <usPrototypeServiceFactory.h>
#include <usModule.h>
#include <usModuleContext.h>

#include <iostream>

namespace us {

struct TestModuleH
{
  virtual ~TestModuleH() {}
};

struct TestModuleH2
{
  virtual ~TestModuleH2() {}
};

class TestProduct : public TestModuleH
{
  // Module* caller;

public:

  TestProduct(Module* /*caller*/)
    //: caller(caller)
  {}

};

class TestProduct2 : public TestProduct, public TestModuleH2
{
public:

  TestProduct2(Module* caller)
    : TestProduct(caller)
  {}

};

class TestModuleHPrototypeServiceFactory : public PrototypeServiceFactory
{
  std::map<long, std::list<TestProduct2*> > fcbind;   // Map calling module with implementation

public:

  InterfaceMap GetService(Module* caller, const ServiceRegistrationBase& /*sReg*/) override
  {
    std::cout << "GetService (prototype) in H" << std::endl;
    TestProduct2* product = new TestProduct2(caller);
    fcbind[caller->GetModuleId()].push_back(product);
    return MakeInterfaceMap<TestModuleH,TestModuleH2>(product);
  }

  void UngetService(Module* caller, const ServiceRegistrationBase& /*sReg*/, const InterfaceMap& service) override
  {
    TestProduct2* product = dynamic_cast<TestProduct2*>(ExtractInterface<TestModuleH>(service));
    delete product;
    fcbind[caller->GetModuleId()].remove(product);
  }

};


class TestModuleHActivator : public ModuleActivator, public ServiceFactory
{
  std::string thisServiceName;
  ServiceRegistration<TestModuleH> factoryService;
  ServiceRegistration<TestModuleH,TestModuleH2> prototypeFactoryService;
  ModuleContext* mc;

  std::map<long, TestProduct*> fcbind;   // Map calling module with implementation
  TestModuleHPrototypeServiceFactory prototypeFactory;

public:

  TestModuleHActivator()
    : thisServiceName(us_service_interface_iid<TestModuleH>())
    , mc(nullptr)
  {}

  void Load(ModuleContext* mc) override
  {
    std::cout << "start in H" << std::endl;
    this->mc = mc;
    factoryService = mc->RegisterService<TestModuleH>(this);
    prototypeFactoryService = mc->RegisterService<TestModuleH,TestModuleH2>(static_cast<ServiceFactory*>(&prototypeFactory));
  }

  void Unload(ModuleContext* /*mc*/) override
  {
    factoryService.Unregister();
  }

  InterfaceMap GetService(Module* caller, const ServiceRegistrationBase& /*sReg*/) override
  {
    std::cout << "GetService in H" << std::endl;
    TestProduct* product = new TestProduct(caller);
    fcbind.insert(std::make_pair(caller->GetModuleId(), product));
    return MakeInterfaceMap<TestModuleH>(product);
  }

  void UngetService(Module* caller, const ServiceRegistrationBase& /*sReg*/, const InterfaceMap& service) override
  {
    TestModuleH* product = ExtractInterface<TestModuleH>(service);
    delete product;
    fcbind.erase(caller->GetModuleId());
  }

};


}

US_EXPORT_MODULE_ACTIVATOR(us::TestModuleHActivator)
