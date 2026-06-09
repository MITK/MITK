/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkSceneFileUtil.h>

#include <itksys/SystemTools.hxx>

bool mitk::EndsWithCaseInsensitive(const std::string& str, const std::string& suffix)
{
  if (str.size() < suffix.size())
    return false;

  const std::string lowerStr = itksys::SystemTools::LowerCase(str);
  const std::string lowerSuffix = itksys::SystemTools::LowerCase(suffix);

  return lowerStr.compare(lowerStr.size() - lowerSuffix.size(), lowerSuffix.size(), lowerSuffix) == 0;
}
