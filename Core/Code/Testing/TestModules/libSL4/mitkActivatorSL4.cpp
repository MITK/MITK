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
#include <mitkModuleContext.h>
#include <mitkServiceRegistration.h>

#include <mitkFooService.h>

namespace mitk {

class ActivatorSL4 :
  public itk::LightObject, public ModuleActivator, public FooService
{
  
public:

  ~ActivatorSL4()
  {
    --m_ReferenceCount;
  }

  void foo()
  {
    MITK_INFO << "TestModuleSL4: Doing foo";
  }
  
  void Load(ModuleContext* context)
  {
    ServiceRegistration registration =
      context->RegisterService<FooService>(this);
    MITK_INFO << "TestModuleSL4: Registered" << registration;
  }
  
  void Unload(ModuleContext* /*context*/)
  {
    //unregister will be done automagically
  }

}; 

}

MITK_EXPORT_MODULE_ACTIVATOR(TestModuleSL4, mitk::ActivatorSL4)
