/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <usModuleActivator.h>
#include <mitkLog.h>
#include "usModulePropsInterface.h"
#include <usServiceTracker.h>
#include <usServiceTrackerCustomizer.h>
#include "usFooService.h"

namespace us {

class ActivatorSL3 :
    public ModuleActivator, public ModulePropsInterface,
    public ServiceTrackerCustomizer<FooService>
{

public:

  ActivatorSL3() : tracker(nullptr), context(nullptr) {}

  ~ActivatorSL3() override
  { delete tracker; }

  void Load(ModuleContext* context) override
  {
    this->context = context;

    InterfaceMap im = MakeInterfaceMap<ModulePropsInterface>(this);
    im.insert(std::make_pair(std::string("ActivatorSL3"), this));
    sr = context->RegisterService(im);
    delete tracker;
    tracker = new FooTracker(context, this);
    tracker->Open();
  }

  void Unload(ModuleContext* /*context*/) override
  {
    tracker->Close();
  }

  const ModulePropsInterface::Properties& GetProperties() const override
  {
    return props;
  }

  FooService* AddingService(const ServiceReferenceType& reference) override
  {
    props["serviceAdded"] = true;
    MITK_INFO << "SL3: Adding reference =" << reference;

    FooService* fooService = context->GetService(reference);
    fooService->foo();
    return fooService;
  }

  void ModifiedService(const ServiceReferenceType& /*reference*/, FooService* /*service*/) override
  {

  }

  void RemovedService(const ServiceReferenceType& reference, FooService* /*service*/) override
  {
    props["serviceRemoved"] = true;
    MITK_INFO << "SL3: Removing reference =" << reference;
  }

private:

  typedef ServiceTracker<FooService> FooTracker;
  FooTracker* tracker;
  ModuleContext* context;

  ServiceRegistrationU sr;

  ModulePropsInterface::Properties props;

};

}

US_EXPORT_MODULE_ACTIVATOR(us::ActivatorSL3)
