/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include <mitkModuleActivator.h>
#include <mitkModulePropsInterface.h>
#include <mitkServiceTracker.h>
#include <mitkServiceTrackerCustomizer.h>
#include <mitkFooService.h>

namespace mitk {

class ActivatorSL3 :
    public itk::LightObject, public ModuleActivator, public ModulePropsInterface,
    public ServiceTrackerCustomizer<FooService*>
{

public:

  ActivatorSL3() : tracker(0), context(0) {}
  
  ~ActivatorSL3()
  { delete tracker; --m_ReferenceCount; }

  void Load(ModuleContext* context)
  {
    this->context = context;

    context->RegisterService("mitk::ActivatorSL3", this);
    delete tracker;
    tracker = new FooTracker(context, this);
    tracker->Open();
  }
  
  void Unload(ModuleContext* /*context*/)
  {
    tracker->Close();
  }

  const ModulePropsInterface::Properties& GetProperties() const
  {
    return props;
  }

  FooService* AddingService(const ServiceReference& reference)
  {
    props["serviceAdded"] = true;
    MITK_INFO << "SL3: Adding reference =" << reference;

    FooService* fooService = context->GetService<FooService>(reference);
    fooService->foo();
    return fooService;
  }
  
  void ModifiedService(const ServiceReference& /*reference*/, FooService* /*service*/)
  {
  
  }
  
  void RemovedService(const ServiceReference& reference, FooService* /*service*/)
  {
    props["serviceRemoved"] = true;
    MITK_INFO << "SL3: Removing reference =" << reference;
  }

private:

  typedef ServiceTracker<FooService*> FooTracker;
  FooTracker* tracker;
  ModuleContext* context;

  ModulePropsInterface::Properties props;

}; 

}

MITK_EXPORT_MODULE_ACTIVATOR(TestModuleSL3, mitk::ActivatorSL3)


