/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkPythonUtil.h>

std::string mitk::PyQuote(const std::string& value)
{
  std::string result;
  result.reserve(value.size() + 2);
  result.push_back('\'');
  for (char c : value)
  {
    if (c == '\\' || c == '\'')
      result.push_back('\\');
    result.push_back(c);
  }
  result.push_back('\'');
  return result;
}
