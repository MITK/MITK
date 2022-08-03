/*============================================================================

  Library: CppMicroServices

  Copyright (c) German Cancer Research Center (DKFZ)
  All rights reserved.

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

    https://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

============================================================================*/


#include <usModuleActivator.h>
#include <usModuleContext.h>
#include <usServiceRegistration.h>

#include <usFooService.h>

US_BEGIN_NAMESPACE

class ActivatorSL4 :
  public ModuleActivator, public FooService
{

public:

  ~ActivatorSL4() override
  {

  }

  void foo() override
  {
    US_INFO << "TestModuleSL4: Doing foo";
  }

  void Load(ModuleContext* context) override
  {
    sr = context->RegisterService<FooService>(this);
    US_INFO << "TestModuleSL4: Registered " << sr;
  }

  void Unload(ModuleContext* /*context*/) override
  {
  }

private:

  ServiceRegistration<FooService> sr;
};

US_END_NAMESPACE

US_EXPORT_MODULE_ACTIVATOR(US_PREPEND_NAMESPACE(ActivatorSL4))
