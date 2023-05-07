/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkNavigationToolStorageTestHelper_h
#define mitkNavigationToolStorageTestHelper_h

#include <mitkNavigationToolStorage.h>

namespace mitk
{
  namespace NavigationToolStorageTestHelper
  {
    //help methods for test tool storages
    MITKIGT_EXPORT mitk::NavigationToolStorage::Pointer CreateTestData_SimpleStorage();

    MITKIGT_EXPORT mitk::NavigationToolStorage::Pointer CreateTestData_StorageWithOneTool();

    MITKIGT_EXPORT mitk::NavigationToolStorage::Pointer CreateTestData_ComplexStorage(std::string toolFilePath, std::string toolSurfacePath1, std::string toolSurfacePath2);
  }
}

#endif
