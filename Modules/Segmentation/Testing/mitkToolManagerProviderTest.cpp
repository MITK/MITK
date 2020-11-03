/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#include "itkLightObject.h"
#include "mitkTestingMacros.h"
#include "mitkToolManagerProvider.h"

#include <usGetModuleContext.h>
#include <usModuleContext.h>
#include <usServiceReference.h>

int mitkToolManagerProviderTest(int, char *[])
{
  MITK_TEST_BEGIN("ToolManagerProvider")

  // at this point the service is already registered by loading the Segmentation module.

  mitk::ToolManagerProvider *service = mitk::ToolManagerProvider::GetInstance();

  MITK_TEST_CONDITION(service != nullptr, "Service was succesfully  called");
  MITK_TEST_CONDITION((service->GetToolManager()) == (mitk::ToolManagerProvider::GetInstance()->GetToolManager()),
                      "Service singleton");
  MITK_TEST_END()
}
