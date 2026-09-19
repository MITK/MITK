/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkIMapperProvider.h>

namespace mitk
{
  IMapperProvider::~IMapperProvider() {}

  std::string IMapperProvider::PROP_BASEDATA_TYPE()
  {
    static std::string s = "org.mitk.IMapperProvider.basedatatype";
    return s;
  }

  std::string IMapperProvider::PROP_SLOT_ID()
  {
    static std::string s = "org.mitk.IMapperProvider.slotid";
    return s;
  }
}
