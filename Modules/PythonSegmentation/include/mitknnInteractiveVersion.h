/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitknnInteractiveVersion_h
#define mitknnInteractiveVersion_h

#include <MitkPythonSegmentationExports.h>

#include <mitkPythonPackageVersion.h>

#include <string>

namespace mitk
{
  namespace nnInteractive
  {
    /** \brief Minimum nnInteractive version this MITK build supports.
     *
     * Used both to build the pip requirement at install time and to detect a
     * too-old package left behind in a reused virtual environment (see
     * CheckInstalledVersion()). Keep in sync with the install requirement.
     */
    inline constexpr const char* MINIMUM_VERSION = "2.5.0";

    /** \brief Exclusive upper bound on the supported nnInteractive version
     *         (the next major release is assumed to break compatibility).
     */
    inline constexpr const char* MAXIMUM_VERSION_EXCLUSIVE = "3.0.0";

    /** \brief Returns the supported nnInteractive versions as a range. */
    inline PythonPackage::VersionRange SupportedVersions()
    {
      return { MINIMUM_VERSION, MAXIMUM_VERSION_EXCLUSIVE };
    }

    /** \brief Compares the installed nnInteractive package against the
     *         supported version range.
     *
     * Creates and activates a transient Python context for the nnInteractive
     * virtual environment. If the context cannot be created (e.g. the virtual
     * environment is missing or the interpreter fails to initialize), the
     * result Status is Unknown.
     *
     * \param[in] checkForUpdate Whether to query PyPI for a newer release.
     * \param[in] distributionName The installed pip distribution to query. The full
     *                    install registers the distribution "nnInteractive"; a
     *                    client-only install registers "nninteractive-client". The
     *                    shared "nnInteractive" import namespace is the same for both,
     *                    so the distribution name (not the import name) must be used
     *                    here for importlib.metadata and the PyPI lookup. Only
     *                    C++-controlled literals are ever passed.
     *
     * \return A PythonPackage::VersionCheckResult; Status is Unknown when the
     *         installed version could not be read (the caller should then not
     *         block or nag).
     *
     * \sa PythonPackage::CheckInstalledVersion()
     */
    MITKPYTHONSEGMENTATION_EXPORT PythonPackage::VersionCheckResult CheckInstalledVersion(bool checkForUpdate = true, const std::string& distributionName = "nnInteractive");
  }
}

#endif
