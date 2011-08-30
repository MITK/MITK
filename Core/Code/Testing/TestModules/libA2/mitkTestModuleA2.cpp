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

#include "mitkTestModuleA2Service.h"

#include <itkLightObject.h>

#include <mitkModuleActivator.h>
#include <mitkModuleContext.h>

namespace mitk {

struct TestModuleA2 : public itk::LightObject, public TestModuleA2Service
{

  TestModuleA2(ModuleContext* mc)
  {
    MITK_INFO << "Registering TestModuleA2Service";
    sr = mc->RegisterService<TestModuleA2Service>(this);
  }

  void Unregister()
  {
    if (sr)
    {
      sr.Unregister();
    }
  }
  
private:

  ServiceRegistration sr;
};

class TestModuleA2Activator : public ModuleActivator
{
public:

  void Load(ModuleContext* context)
  {
    s = new TestModuleA2(context);
  }

  void Unload(ModuleContext* /*context*/)
  {
    s->Unregister();
  }

private:

  itk::SmartPointer<TestModuleA2> s;
};

}

MITK_EXPORT_MODULE_ACTIVATOR(TestModuleA2, mitk::TestModuleA2Activator)

