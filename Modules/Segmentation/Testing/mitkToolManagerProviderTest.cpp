/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/
#include "mitkTestingMacros.h"
#include "mitkToolManagerProvider.h"
#include "itkLightObject.h"

#include <usGetModuleContext.h>
#include "mitkModuleContext.h"
#include "mitkServiceReference.h"

#include "mitkGlobalInteraction.h"

class mitkToolManagerProviderTestClass {
public:
  static void RegisterService(){
    mitk::GetModuleContext()->RegisterService<mitk::ToolManagerProvider>(mitk::ToolManagerProvider::New().GetPointer());
  }

};

int mitkToolManagerProviderTest(int, char* [])
{
  MITK_TEST_BEGIN("ToolManagerProvider")

    mitk::GlobalInteraction::GetInstance()->Initialize("global");

    mitkToolManagerProviderTestClass::RegisterService();

    mitk::ToolManagerProvider* service = mitk::ToolManagerProvider::GetInstance();



  MITK_TEST_CONDITION(service!=NULL,"Service was succesfully  called");
  MITK_TEST_CONDITION((service->GetToolManager()) == (service->GetToolManager()), "Service singleton");
  MITK_TEST_END()
}
