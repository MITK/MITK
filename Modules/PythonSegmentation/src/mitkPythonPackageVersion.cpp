/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkPythonPackageVersion.h>

#include <mitkException.h>
#include <mitkLog.h>
#include <mitkPythonContext.h>
#include <mitkPythonUtil.h>

#include <sstream>

mitk::PythonPackage::VersionCheckResult mitk::PythonPackage::CheckInstalledVersion(PythonContext& context, const std::string& distributionName, const VersionRange& range, bool checkForUpdate)
{
  VersionCheckResult result;

  const auto quotedDistribution = PyQuote(distributionName);
  const auto quotedMinimum = PyQuote(range.Minimum);

  // Read the installed version with the standard library alone, so a missing
  // third-party 'packaging' never hides an installed package behind an Unknown
  // verdict (a client-only venv has no torch to pull 'packaging' in; the install
  // requests it explicitly, but guard it regardless). 'packaging' is only needed
  // for the comparisons that follow. Reach out to PyPI only once the installed
  // version is known to be supported, so a too-old package is flagged instantly
  // and a missing network never delays the result.
  std::ostringstream pyCommands; pyCommands
    << "mitk_pkg_installed = ''\n"
    << "mitk_pkg_latest = ''\n"
    << "mitk_pkg_below_min = False\n"
    << "mitk_pkg_update_available = False\n"
    << "try:\n"
    << "    from importlib.metadata import version\n"
    << "    mitk_pkg_installed = version(" << quotedDistribution << ")\n"
    << "except Exception:\n"
    << "    mitk_pkg_installed = ''\n"
    << "if mitk_pkg_installed:\n"
    << "    try:\n"
    << "        from packaging.version import Version\n"
    << "        mitk_pkg_below_min = Version(mitk_pkg_installed) < Version(" << quotedMinimum << ")\n";

  if (checkForUpdate)
  {
    pyCommands
      << "        if not mitk_pkg_below_min:\n"
      << "            try:\n"
      << "                from packaging.specifiers import SpecifierSet\n"
      << "                import urllib.request, json\n"
      << "                with urllib.request.urlopen('https://pypi.org/pypi/' + " << quotedDistribution << " + '/json', timeout=5) as _r:\n"
      << "                    _data = json.load(_r)\n"
      << "                _spec = SpecifierSet(" << PyQuote(range.ToPipSpecifier()) << ")\n"
      // Consider only releases with installable, non-yanked files. PyPI's
      // 'releases' map keeps every version ever registered, including yanked or
      // fileless ones that pip would never install, so filtering by the version
      // string alone could advertise an update that cannot actually be had.
      << "                _cands = [v for v, _files in _data['releases'].items()\n"
      << "                          if _files and not all(_f.get('yanked') for _f in _files)\n"
      << "                          and _spec.contains(v, prereleases=False)]\n"
      << "                if _cands:\n"
      << "                    mitk_pkg_latest = str(max(_cands, key=Version))\n"
      << "                    mitk_pkg_update_available = Version(mitk_pkg_latest) > Version(mitk_pkg_installed)\n"
      << "            except Exception:\n"
      << "                mitk_pkg_latest = ''\n"
      << "                mitk_pkg_update_available = False\n";
  }

  pyCommands
    // A missing 'packaging' or an unparsable version string must not pass for
    // UpToDate: clearing mitk_pkg_installed makes the result inconclusive, so the
    // C++ below reports Unknown rather than silently skipping the version gate.
    << "    except Exception:\n"
    << "        mitk_pkg_installed = ''\n";

  try
  {
    context.Execute(pyCommands.str());
  }
  catch (...)
  {
    return result; // Status stays Unknown.
  }

  result.Installed = context.GetVariableAsString("mitk_pkg_installed").value_or("");
  result.Latest = context.GetVariableAsString("mitk_pkg_latest").value_or("");

  if (result.Installed.empty())
    return result; // Unknown: could not read the installed version.

  if (context.GetVariableAsBool("mitk_pkg_below_min").value_or(false))
    result.Status = VersionStatus::BelowMinimum;
  else if (context.GetVariableAsBool("mitk_pkg_update_available").value_or(false))
    result.Status = VersionStatus::UpdateAvailable;
  else
    result.Status = VersionStatus::UpToDate;

  return result;
}

mitk::PythonPackage::VersionCheckResult mitk::PythonPackage::CheckInstalledVersion(const std::string& venvName, const std::string& distributionName, const VersionRange& range, bool checkForUpdate)
{
  try
  {
    PythonContext context(venvName);

    // Metadata-only: read the installed version and query PyPI without importing
    // NumPy or the MITK module, so this check never maps a venv native library
    // (which on Linux would then read back as "loaded" and block the update).
    context.Activate(false);

    return CheckInstalledVersion(context, distributionName, range, checkForUpdate);
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
    MITK_ERROR << "Unexpected error while checking the installed version of " << distributionName << ".";
    return {};
  }
}
