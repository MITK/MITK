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

  propertyDescriptions->AddDescription("propertyName1", "description1a");
  propertyDescriptions->AddDescription("propertyName1", "description1b");
  std::string description1 = propertyDescriptions->GetDescription("propertyName1");

  MITK_TEST_CONDITION(
    description1 == "description1a",
    "Get description of \"propertyName1\"");

  propertyDescriptions->AddDescription("propertyName1", "description1b", "", true);
  description1 = propertyDescriptions->GetDescription("propertyName1");

  MITK_TEST_CONDITION(
    description1 == "description1b",
    "Get overwritten description of \"propertyName1\"");

  propertyDescriptions->AddDescription("propertyName1", "description1c", "className");
  std::string description2 = propertyDescriptions->GetDescription("propertyName1", "className");
  description1 = propertyDescriptions->GetDescription("propertyName1");

  MITK_TEST_CONDITION(
    description1 == "description1b" && description2 == "description1c",
    "Get description of \"propertyName1\" restricted to \"className\"");

  MITK_TEST_END();
}
