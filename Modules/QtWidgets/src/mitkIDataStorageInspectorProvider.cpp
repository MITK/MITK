/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkIDataStorageInspectorProvider.h"

namespace mitk
{
  IDataStorageInspectorProvider::~IDataStorageInspectorProvider() {}

  std::string mitk::IDataStorageInspectorProvider::PROP_INSPECTOR_ID()
  {
    static std::string s = "org.mitk.IDataStorageInspectorProvider.inspectorid";
    return s;
  }

}
