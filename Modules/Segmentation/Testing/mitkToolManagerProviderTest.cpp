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
#include <usModuleContext.h>
#include <usServiceReference.h>



int mitkToolManagerProviderTest(int, char* [])
{
  MITK_TEST_BEGIN("ToolManagerProvider")


    //at this point the service is already registered by loading the Segmentation module.

    mitk::ToolManagerProvider* service = mitk::ToolManagerProvider::GetInstance();

  MITK_TEST_CONDITION(service!=NULL,"Service was succesfully  called");
  MITK_TEST_CONDITION((service->GetToolManager()) == (mitk::ToolManagerProvider::GetInstance()->GetToolManager()), "Service singleton");
  MITK_TEST_END()
}
