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
#include <mitkIPropertyFilters.h>
#include <mitkTestingMacros.h>

int mitkPropertyFiltersTest(int, char* [])
{
  MITK_TEST_BEGIN("mitkPropertyFiltersTest");

  mitk::IPropertyFilters* propertyFilters = mitk::CoreServices::GetPropertyFilters();
  MITK_TEST_CONDITION_REQUIRED(propertyFilters != NULL, "Get property filters service");

  MITK_TEST_END();
}
