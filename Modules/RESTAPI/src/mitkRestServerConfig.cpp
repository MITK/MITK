/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkRestServerConfig.h>

namespace mitk
{
  std::string GetRestApiDocumentationUrl(int major, int minor, int patch)
  {
    const std::string segment = (99 == patch)
      ? "nightly"
      : std::to_string(major) + "." + std::to_string(minor);

    return "https://docs.mitk.org/" + segment + "/MITKRESTAPISpec.html";
  }
}
