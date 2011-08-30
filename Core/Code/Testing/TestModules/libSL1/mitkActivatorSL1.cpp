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


#include <itkLightObject.h>

#include <mitkModuleActivator.h>
#include <mitkModulePropsInterface.h>

#include <mitkServiceTracker.h>
#include <mitkServiceTrackerCustomizer.h>

#include "mitkFooService.h"

namespace mitk {

class ActivatorSL1 :
    public itk::LightObject, public ModuleActivator, public ModulePropsInterface,
    public ServiceTrackerCustomizer<FooService*>
{
  
public:

  ActivatorSL1()
    : tracker(0), context(0)
  {
  
  }
  
  ~ActivatorSL1()
  {
    delete tracker;
    --m_ReferenceCount;
  }

  void Load(ModuleContext* context)
  {
    this->context = context;

    context->RegisterService("mitk::ActivatorSL1", this);

    delete tracker;
    tracker = new FooTracker(context, this);
    tracker->Open();
  }

  void Unload(ModuleContext* /*context*/)
  {
    tracker->Close();
  }

  const Properties& GetProperties() const
  {
    return props;
  }

  FooService* AddingService(const ServiceReference& reference)
  {
    props["serviceAdded"] = true;

    FooService* fooService = context->GetService<FooService>(reference);
    fooService->foo();
    return fooService;
  }

  void ModifiedService(const ServiceReference& /*reference*/, FooService* /*service*/)
  {}

  void RemovedService(const ServiceReference& /*reference*/, FooService* /*service*/)
  {
    props["serviceRemoved"] = true;
  }

private:

  ModulePropsInterface::Properties props;

  typedef ServiceTracker<FooService*> FooTracker;
  
  FooTracker* tracker;
  ModuleContext* context;

}; // ActivatorSL1

} // end namespace mitk

MITK_EXPORT_MODULE_ACTIVATOR(TestModuleSL1, mitk::ActivatorSL1)


