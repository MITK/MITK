/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitknnInteractiveVersion.h>

mitk::PythonPackage::VersionCheckResult mitk::nnInteractive::CheckInstalledVersion(PythonContext& context, bool checkForUpdate, const std::string& distributionName)
{
  return PythonPackage::CheckInstalledVersion(context, distributionName, SupportedVersions(), checkForUpdate);
}

mitk::PythonPackage::VersionCheckResult mitk::nnInteractive::CheckInstalledVersion(bool checkForUpdate, const std::string& distributionName)
{
  return PythonPackage::CheckInstalledVersion("nnInteractive", distributionName, SupportedVersions(), checkForUpdate);
}
