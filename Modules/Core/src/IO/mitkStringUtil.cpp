/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkStringUtil.h>

#include <cctype>

bool mitk::EndsWithCaseInsensitive(const std::string& str, const std::string& suffix)
{
  if (str.size() < suffix.size())
    return false;

  // Compare only the trailing suffix.size() characters. std::tolower must be given
  // an unsigned char to avoid undefined behavior on negative char values.
  const auto offset = str.size() - suffix.size();

  for (std::string::size_type i = 0; i < suffix.size(); ++i)
  {
    if (std::tolower(static_cast<unsigned char>(str[offset + i])) !=
        std::tolower(static_cast<unsigned char>(suffix[i])))
      return false;
  }

  return true;
}
