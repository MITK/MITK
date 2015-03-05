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
