/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkRestServerConfig.h>

#include <iomanip>
#include <sstream>

namespace
{
  // MITK's development-build sentinel: CMake sets the patch level to 99 for
  // unreleased builds (and appends the git revision to the version string).
  constexpr int devBuildPatchSentinel = 99;
}

namespace mitk
{
  std::string GetRestApiDocumentationUrl(int major, int minor, int patch)
  {
    // Released docs live under a zero-padded YYYY.MM path on docs.mitk.org
    // (e.g. /2026.06/); a single-digit minor without the pad 404s. Development
    // builds (patch level 99) publish under /nightly/ instead.
    std::string segment;
    if (devBuildPatchSentinel == patch)
    {
      segment = "nightly";
    }
    else
    {
      std::ostringstream oss;
      oss << major << '.' << std::setw(2) << std::setfill('0') << minor;
      segment = oss.str();
    }

    return "https://docs.mitk.org/" + segment + "/MITKRESTAPISpec.html";
  }
}
