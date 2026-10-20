/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkPythonPackageVersion.h>

#include "mitkPythonWorkerModules.h"

#include <mitkExceptionMacro.h>
#include <mitkLog.h>
#include <mitkPythonHelper.h>
#include <mitkPythonWorker.h>

mitk::PythonPackage::PackageProbe::PackageProbe(const std::string& venvName)
{
  const auto python = PythonHelper::GetVirtualEnvExecutablePath(venvName);

  if (python.empty())
    mitkThrow() << "The virtual environment " << venvName << " has no Python interpreter.";

  m_Worker = std::make_unique<PythonWorker>(python, std::vector<PythonWorker::Module> {
    ReadWorkerModule("package_checks.py")
  }, venvName);
}

mitk::PythonPackage::PackageProbe::~PackageProbe() = default;

bool mitk::PythonPackage::PackageProbe::HasModule(const std::string& moduleName)
{
  if (m_Worker == nullptr)
    mitkThrow() << "The package probe was ended.";

  return m_Worker->Call("package_checks.has_module", { { "name", moduleName } }).get<bool>();
}

bool mitk::PythonPackage::PackageProbe::HasDistribution(const std::string& distributionName)
{
  if (m_Worker == nullptr)
    mitkThrow() << "The package probe was ended.";

  return m_Worker->Call("package_checks.has_distribution", { { "name", distributionName } }).get<bool>();
}

mitk::PythonPackage::VersionCheckResult mitk::PythonPackage::PackageProbe::CheckVersion(const std::string& distributionName, const VersionRange& range, bool checkForUpdate)
{
  VersionCheckResult result;

  if (m_Worker == nullptr)
    return result;

  nlohmann::json check;

  try
  {
    check = m_Worker->Call("package_checks.check_version", {
      { "distribution", distributionName },
      { "minimum", range.Minimum },
      { "specifier", range.ToPipSpecifier() },
      { "check_for_update", checkForUpdate }
    });
  }
  catch (const Exception& e)
  {
    MITK_ERROR << "Could not check the installed version of " << distributionName << ": " << e.GetDescription();
    return result;
  }

  result.Installed = check.value("installed", std::string());
  result.Latest = check.value("latest", std::string());

  if (result.Installed.empty())
    return result; // Unknown: could not read the installed version.

  if (check.value("below_minimum", false))
    result.Status = VersionStatus::BelowMinimum;
  else if (check.value("update_available", false))
    result.Status = VersionStatus::UpdateAvailable;
  else
    result.Status = VersionStatus::UpToDate;

  return result;
}

void mitk::PythonPackage::PackageProbe::End()
{
  m_Worker.reset();
}

mitk::PythonPackage::VersionCheckResult mitk::PythonPackage::CheckInstalledVersion(const std::string& venvName, const std::string& distributionName, const VersionRange& range, bool checkForUpdate)
{
  try
  {
    PackageProbe probe(venvName);
    return probe.CheckVersion(distributionName, range, checkForUpdate);
  }
  catch (const Exception& e)
  {
    MITK_ERROR << e.GetDescription();
    return {};
  }
}
