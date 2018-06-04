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