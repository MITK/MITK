/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#include <mitkNavigationToolStorage.h>
namespace mitk {
  class MITKIGT_EXPORT NavigationToolStorageTestHelper
  {
  public:
  //help methods for test tool storages
  static mitk::NavigationToolStorage::Pointer CreateTestData_SimpleStorage();

  static mitk::NavigationToolStorage::Pointer CreateTestData_StorageWithOneTool();

  static mitk::NavigationToolStorage::Pointer CreateTestData_ComplexStorage(std::string toolFilePath, std::string toolSurfacePath1, std::string toolSurfacePath2);
  };
}
