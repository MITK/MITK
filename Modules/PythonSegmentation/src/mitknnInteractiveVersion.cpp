/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitknnInteractiveVersion.h>

#include <mitkException.h>
#include <mitkLog.h>
#include <mitkPythonContext.h>

#include <sstream>

mitk::nnInteractive::VersionCheckResult mitk::nnInteractive::CheckInstalledVersion(PythonContext& context, bool checkForUpdate, const std::string& distributionName)
{
  VersionCheckResult result;

  const std::string spec = std::string(">=") + MINIMUM_VERSION + ",<" + MAXIMUM_VERSION_EXCLUSIVE;

  // Determine the offline verdict (installed vs. minimum) first and only reach
  // out to PyPI when the installed version is already supported, so a too-old
  // package is flagged instantly and a missing network never delays the result.
  // packaging ships with pip/torch, so the imports are expected to succeed; any
  // failure leaves nni_installed empty and the caller treats it as Unknown.
  std::ostringstream pyCommands; pyCommands
    << "nni_installed = ''\n"
    << "nni_latest = ''\n"
    << "nni_below_min = False\n"
    << "nni_update_available = False\n"
    << "try:\n"
    << "    from importlib.metadata import version\n"
    << "    from packaging.version import Version\n"
    << "    nni_installed = version('" << distributionName << "')\n"
    << "    nni_below_min = Version(nni_installed) < Version('" << MINIMUM_VERSION << "')\n";

  if (checkForUpdate)
  {
    pyCommands
      << "    if not nni_below_min:\n"
      << "        try:\n"
      << "            from packaging.specifiers import SpecifierSet\n"
      << "            import urllib.request, json\n"
      << "            with urllib.request.urlopen('https://pypi.org/pypi/" << distributionName << "/json', timeout=5) as _r:\n"
      << "                _data = json.load(_r)\n"
      << "            _spec = SpecifierSet('" << spec << "')\n"
      // Consider only releases with installable, non-yanked files. PyPI's
      // 'releases' map keeps every version ever registered, including yanked or
      // fileless ones that pip would never install, so filtering by the version
      // string alone could advertise an update that cannot actually be had.
      << "            _cands = [v for v, _files in _data['releases'].items()\n"
      << "                      if _files and not all(_f.get('yanked') for _f in _files)\n"
      << "                      and _spec.contains(v, prereleases=False)]\n"
      << "            if _cands:\n"
      << "                nni_latest = str(max(_cands, key=Version))\n"
      << "                nni_update_available = Version(nni_latest) > Version(nni_installed)\n"
      << "        except Exception:\n"
      << "            nni_latest = ''\n"
      << "            nni_update_available = False\n";
  }

  pyCommands
    << "except Exception:\n"
    << "    nni_installed = ''\n";

  try
  {
    context.Execute(pyCommands.str());
  }
  catch (...)
  {
    return result; // Status stays Unknown.
  }

  result.Installed = context.GetVariableAsString("nni_installed").value_or("");
  result.Latest = context.GetVariableAsString("nni_latest").value_or("");

  if (result.Installed.empty())
    return result; // Unknown: could not read the installed version.

  if (context.GetVariableAsBool("nni_below_min").value_or(false))
    result.Status = VersionStatus::BelowMinimum;
  else if (context.GetVariableAsBool("nni_update_available").value_or(false))
    result.Status = VersionStatus::UpdateAvailable;
  else
    result.Status = VersionStatus::UpToDate;

  return result;
}

mitk::nnInteractive::VersionCheckResult mitk::nnInteractive::CheckInstalledVersion(bool checkForUpdate, const std::string& distributionName)
{
  try
  {
    PythonContext context("nnInteractive");
    context.Activate();

    return CheckInstalledVersion(context, checkForUpdate, distributionName);
  }
  catch (const Exception& e)
  {
    MITK_ERROR << e.GetDescription();
    return {};
  }
  catch (...)
  {
    // The PythonContext constructor can throw a non-mitk exception (e.g. a
    // std::runtime_error from initializing the embedded interpreter for the
    // first time). Swallow it here so it never escapes into a Qt slot.
    MITK_ERROR << "Unexpected error while checking the nnInteractive version.";
    return {};
  }
}
