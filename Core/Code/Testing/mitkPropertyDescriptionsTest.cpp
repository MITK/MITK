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

#include <mitkCoreServices.h>
#include <mitkIPropertyDescriptions.h>
#include <mitkTestingMacros.h>

int mitkPropertyDescriptionsTest(int, char* [])
{
  MITK_TEST_BEGIN("mitkPropertyDescriptionsTest");

  mitk::IPropertyDescriptions* propertyDescriptions = mitk::CoreServices::GetPropertyDescriptions();
  MITK_TEST_CONDITION_REQUIRED(propertyDescriptions != NULL, "Get property descriptions service");

  MITK_TEST_END();
}
