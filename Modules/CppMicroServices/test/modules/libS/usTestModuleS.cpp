/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "../../usServiceControlInterface.h"

#include "usTestModuleSService0.h"
#include "usTestModuleSService1.h"
#include "usTestModuleSService2.h"
#include "usTestModuleSService3.h"

#include <usServiceRegistration.h>
#include <usModuleContext.h>
#include <usModuleActivator.h>


namespace us {

class TestModuleS : public ServiceControlInterface,
                    public TestModuleSService0,
                    public TestModuleSService1,
                    public TestModuleSService2,
                    public TestModuleSService3
{

public:

  TestModuleS(ModuleContext* mc)
    : mc(mc)
  {
    for(int i = 0; i <= 3; ++i)
    {
      servregs.push_back(ServiceRegistrationU());
    }
    sreg = mc->RegisterService<TestModuleSService0>(this);
    sciReg = mc->RegisterService<ServiceControlInterface>(this);
  }

  virtual const char* GetNameOfClass() const
  {
    return "TestModuleS";
  }

  void ServiceControl(int offset, const std::string& operation, int ranking) override
  {
    if (0 <= offset && offset <= 3)
    {
      if (operation == "register")
      {
        if (!servregs[offset])
        {
          std::stringstream servicename;
          servicename << SERVICE << offset;
          InterfaceMap ifm;
          ifm.insert(std::make_pair(servicename.str(), static_cast<void*>(this)));
          ServiceProperties props;
          props.insert(std::make_pair(ServiceConstants::SERVICE_RANKING(), Any(ranking)));
          servregs[offset] = mc->RegisterService(ifm, props);
        }
      }
      if (operation == "unregister")
      {
        if (servregs[offset])
        {
          ServiceRegistrationU sr1 = servregs[offset];
          sr1.Unregister();
          servregs[offset] = 0;
        }
      }
    }
  }

  void Unregister()
  {
    if (sreg)
    {
      sreg.Unregister();
    }
    if (sciReg)
    {
      sciReg.Unregister();
    }
  }

private:

  static const std::string SERVICE; // = "us::TestModuleSService"

  ModuleContext* mc;
  std::vector<ServiceRegistrationU> servregs;
  ServiceRegistration<TestModuleSService0> sreg;
  ServiceRegistration<ServiceControlInterface> sciReg;
};

const std::string TestModuleS::SERVICE = "us::TestModuleSService";

class TestModuleSActivator : public ModuleActivator
{

public:

  TestModuleSActivator() : s(nullptr) {}
  ~TestModuleSActivator() override { delete s; }

  void Load(ModuleContext* context) override
  {
    s = new TestModuleS(context);
  }

  void Unload(ModuleContext* /*context*/) override
  {
    }

private:

  TestModuleS* s;

};

}

US_EXPORT_MODULE_ACTIVATOR(us::TestModuleSActivator)
