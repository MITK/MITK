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
#include <mitkIPropertyExtensions.h>
#include <mitkTestingMacros.h>

int mitkPropertyExtensionsTest(int, char* [])
{
  MITK_TEST_BEGIN("mitkPropertyExtensionsTest");

  mitk::IPropertyExtensions* propertyExtensions = mitk::CoreServices::GetPropertyExtensions();
  MITK_TEST_CONDITION_REQUIRED(propertyExtensions != NULL, "Get property extensions service");

  MITK_TEST_END();
}
