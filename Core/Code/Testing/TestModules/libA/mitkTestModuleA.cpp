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

#include "mitkTestModuleAService.h"

#include <itkLightObject.h>

#include <mitkModuleActivator.h>
#include <mitkModuleContext.h>

#include <mitkLogMacros.h>

namespace mitk {

struct TestModuleA : public itk::LightObject, public TestModuleAService
{

  TestModuleA(ModuleContext* mc) : m_UnloadCalled(0)
  {
    MITK_INFO << "Registering TestModuleAService";
    mc->RegisterService<TestModuleAService>(this);
  }

  void SetUnloadedFlag(bool* flag)
  {
    m_UnloadCalled = flag;
  }

  bool* m_UnloadCalled;

};

class TestModuleAActivator : public ModuleActivator
{
public:

  void Load(ModuleContext* context)
  {
    s = new TestModuleA(context);
  }

  void Unload(ModuleContext*)
  {
    if (s->m_UnloadCalled)
    {
      *(s->m_UnloadCalled) = true;
    }
  }

private:

  itk::SmartPointer<TestModuleA> s;
};

}

MITK_EXPORT_MODULE_ACTIVATOR(TestModuleA, mitk::TestModuleAActivator)

