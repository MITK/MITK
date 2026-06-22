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

#include <string>

namespace mitk
{
  class PythonContext;

  namespace nnInteractive
  {
    /** \brief Outcome of comparing the installed nnInteractive package against
     *         the version range this MITK build supports.
     *
     * \sa CheckInstalledVersion()
     */
    enum class VersionStatus
    {
      Unknown,         /**< \brief The installed or latest version could not be determined. */
      BelowMinimum,    /**< \brief Installed version is older than MINIMUM_VERSION (incompatible). */
      UpdateAvailable, /**< \brief A newer in-range release exists on PyPI than the one installed. */
      UpToDate         /**< \brief Installed version is supported and no newer in-range release is known. */
    };

    /** \brief Result of CheckInstalledVersion(): the verdict plus the version
     *         strings it was derived from (empty when not determined).
     */
    struct VersionCheckResult
    {
      VersionStatus Status = VersionStatus::Unknown;
      std::string Installed;
      std::string Latest;
    };

    /** \brief Minimum nnInteractive version this MITK build supports.
     *
     * Used both to build the pip requirement at install time and to detect a
     * too-old package left behind in a reused virtual environment (see
     * CheckInstalledVersion()). Keep in sync with the install requirement.
     */
    inline constexpr const char* MINIMUM_VERSION = "2.4.0";

    /** \brief Exclusive upper bound on the supported nnInteractive version
     *         (the next major release is assumed to break compatibility).
     */
    inline constexpr const char* MAXIMUM_VERSION_EXCLUSIVE = "3.0.0";

    /** \brief Compares the installed nnInteractive package against the
     *         supported version range, using the given Python context.
     *
     * Reads the installed version via importlib.metadata and compares it with
     * MINIMUM_VERSION using packaging's version semantics. When \p checkForUpdate
     * is \c true and the installed version meets the minimum, it additionally
     * queries PyPI (best-effort, short timeout) for the newest release within the
     * supported range to spot an available update; the query is skipped when the
     * installed version is already below the minimum (it is going to be
     * reinstalled anyway) and on any network failure, so the offline verdict is
     * always fast and reliable. Pass \c false to do the offline minimum check
     * only (the caller has already performed the update check this run, so the
     * network round-trip would be wasted).
     *
     * A successful update query sets VersionCheckResult::Latest to the newest
     * in-range release (equal to the installed version when it is already the
     * newest); a network failure leaves it empty. So an UpToDate result with a
     * non-empty Latest means PyPI was actually reached, an empty one means it
     * was not, which lets a caller distinguish a confirmed up-to-date verdict
     * from an offline check.
     *
     * \param[in] context An activated Python context bound to the nnInteractive
     *                    virtual environment.
     * \param[in] checkForUpdate Whether to query PyPI for a newer release.
     *
     * \return A VersionCheckResult; Status is Unknown when the installed version
     *         could not be read (the caller should then not block or nag).
     *
     * \sa VersionStatus
     */
    MITKPYTHONSEGMENTATION_EXPORT VersionCheckResult CheckInstalledVersion(PythonContext& context, bool checkForUpdate = true);

    /** \brief Convenience overload for callers without a Python context.
     *
     * Creates and activates a transient Python context for the nnInteractive
     * virtual environment, then forwards to the context-taking overload. If the
     * context cannot be created (e.g. the virtual environment is missing or the
     * interpreter fails to initialize), the result Status is Unknown.
     *
     * \param[in] checkForUpdate Whether to query PyPI for a newer release.
     *
     * \return A VersionCheckResult (see the context-taking overload).
     *
     * \sa CheckInstalledVersion(PythonContext&, bool)
     */
    MITKPYTHONSEGMENTATION_EXPORT VersionCheckResult CheckInstalledVersion(bool checkForUpdate = true);
  }
}

#endif
