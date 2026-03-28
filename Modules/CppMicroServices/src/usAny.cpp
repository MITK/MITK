/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <usAny.h>

namespace us {

std::string any_value_to_string(const Any& any)
{
  return any.ToString();
}

std::string any_value_to_json(const Any& val)
{
  return val.ToJSON();
}

std::string any_value_to_json(const std::string& val)
{
  return '"' + val + '"';
}

std::string any_value_to_json(bool val)
{
  return val ? "true" : "false";
}

}
