/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPythonPackageVersion_h
#define mitkPythonPackageVersion_h

#include <MitkPythonSegmentationExports.h>

#include <memory>
#include <string>

namespace mitk
{
  class PythonWorker;

  namespace PythonPackage
  {
    /** \brief Outcome of comparing an installed package against the version
     *         range this MITK build supports.
     *
     * \sa CheckInstalledVersion()
     */
    enum class VersionStatus
    {
      Unknown,         /**< \brief The installed or latest version could not be determined. */
      BelowMinimum,    /**< \brief Installed version is older than the minimum (incompatible). */
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

    /** \brief The versions of a package this MITK build supports.
     *
     * The minimum is also what a too-old package left behind in a reused virtual
     * environment is detected by, so it has to match the requirement the install
     * spec requests.
     */
    struct VersionRange
    {
      std::string Minimum;          /**< \brief Oldest supported version. */
      std::string MaximumExclusive; /**< \brief First version that is not supported (the next major
                                                release is assumed to break compatibility). */

      /** \brief Returns the range as a pip version specifier, ">=Minimum,<MaximumExclusive". */
      std::string ToPipSpecifier() const
      {
        return ">=" + this->Minimum + ",<" + this->MaximumExclusive;
      }
    };

    /** \brief Answers questions about the packages of a virtual environment.
     *
     * Runs the interpreter of the environment in a PythonWorker, so nothing of
     * the environment is imported into MITK, and reads metadata only, so no
     * native library of a package is loaded. One worker serves all questions
     * for the lifetime of the probe, or until End().
     *
     * The running worker counts as using the environment (see
     * mitk::GetVirtualEnvUse()), so end the probe before updating or removing it.
     */
    class MITKPYTHONSEGMENTATION_EXPORT PackageProbe
    {
    public:
      /** \throw mitk::Exception if the environment has no interpreter or it cannot be started. */
      explicit PackageProbe(const std::string& venvName);
      ~PackageProbe();

      PackageProbe(const PackageProbe&) = delete;
      PackageProbe& operator=(const PackageProbe&) = delete;

      /** \brief Whether a top-level module can be imported, without importing it.
       *
       * \throw mitk::Exception if the probe was ended or its worker failed.
       */
      bool HasModule(const std::string& moduleName);

      /** \brief Whether a pip distribution is installed, by its metadata.
       *
       * \throw mitk::Exception if the probe was ended or its worker failed.
       */
      bool HasDistribution(const std::string& distributionName);

      /** \brief Compares an installed package against a supported version range.
       *
       * Reads the installed version via importlib.metadata and compares it with
       * the minimum of \p range using packaging's version semantics. When
       * \p checkForUpdate is \c true and the installed version meets the minimum,
       * it additionally queries PyPI (best-effort, short timeout) for the newest
       * release within the range to spot an available update; the query is
       * skipped when the installed version is already below the minimum (it is
       * going to be reinstalled anyway) and on any network failure, so the offline
       * verdict is always fast and reliable. Pass \c false to do the offline
       * minimum check only (the caller has already performed the update check
       * this run, so the network round-trip would be wasted).
       *
       * A successful update query sets VersionCheckResult::Latest to the newest
       * in-range release (equal to the installed version when it is already the
       * newest); a network failure leaves it empty. So an UpToDate result with a
       * non-empty Latest means PyPI was actually reached, an empty one means it
       * was not, which lets a caller distinguish a confirmed up-to-date verdict
       * from an offline check.
       *
       * \param[in] distributionName The installed pip distribution to query. This
       *                    is the name pip knows, which can differ from the
       *                    import name.
       * \param[in] range The supported versions.
       * \param[in] checkForUpdate Whether to query PyPI for a newer release.
       *
       * \return A VersionCheckResult; Status is Unknown when the installed version
       *         could not be read or the worker failed (the caller should then not
       *         block or nag).
       *
       * \sa VersionStatus
       */
      VersionCheckResult CheckVersion(const std::string& distributionName, const VersionRange& range, bool checkForUpdate = true);

      /** \brief Ends the worker of the probe. Further questions throw. */
      void End();

    private:
      std::unique_ptr<PythonWorker> m_Worker;
    };

    /** \brief Checks the installed version of a package with a probe of its own.
     *
     * See PackageProbe::CheckVersion(). If the probe cannot be started (e.g. the
     * virtual environment is missing or broken), the result Status is Unknown.
     *
     * \param[in] venvName Name of the virtual environment the package is installed in.
     * \param[in] distributionName The installed pip distribution to query.
     * \param[in] range The supported versions.
     * \param[in] checkForUpdate Whether to query PyPI for a newer release.
     */
    MITKPYTHONSEGMENTATION_EXPORT VersionCheckResult CheckInstalledVersion(const std::string& venvName, const std::string& distributionName, const VersionRange& range, bool checkForUpdate = true);
  }
}

#endif
