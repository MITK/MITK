/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPythonPackageUpdatePrompt_h
#define mitkPythonPackageUpdatePrompt_h

#include <MitkPythonSegmentationUIExports.h>

#include <mitkPipPackageInfo.h>
#include <mitkPythonPackageVersion.h>
#include <mitkVirtualEnvUse.h>

#include <QString>

#include <string>

class QWidget;

namespace mitk
{
  class Exception;

  namespace PythonPackage
  {
    /** \brief The user's response to the version-update prompt. */
    enum class UpdatePromptChoice
    {
      Update,            /**< \brief Run the in-place update now. */
      ContinueInstalled, /**< \brief Keep using the installed version. */
      Cancel             /**< \brief Do nothing / abort the surrounding flow. */
    };

    /** \brief Shows the version-status dialog of a Python package and returns
     *         the user's choice.
     *
     * Handles the BelowMinimum and UpdateAvailable verdicts (read from
     * \p result.Status); passing any other status is a usage error. The wording
     * and the available buttons adapt to:
     *
     * - \p use: an environment in use cannot be updated in place. The dialog
     *   then offers no "Update now" and says what frees it (see
     *   DescribeHowToFree()).
     * - \p inInitFlow: shown during initialization, where declining still proceeds
     *   with the working installed version (offers "Continue with installed", which
     *   maps to ContinueInstalled). When \c false (the preferences page), declining
     *   simply closes the dialog.
     *
     * \param[in] parent Dialog parent (may be \c nullptr).
     * \param[in] packageName The name of the package as the user knows it, e.g. "nnInteractive".
     * \param[in] supportedVersions The versions this MITK build supports.
     * \param[in] result Version check verdict; only BelowMinimum / UpdateAvailable.
     * \param[in] use How this process uses the virtual environment of the package.
     * \param[in] inInitFlow Whether the prompt is shown during initialization.
     *
     * \return The user's choice. BelowMinimum never yields ContinueInstalled.
     */
    MITKPYTHONSEGMENTATIONUI_EXPORT UpdatePromptChoice ShowUpdatePrompt(QWidget* parent, const std::string& packageName, const VersionRange& supportedVersions, const VersionCheckResult& result, VirtualEnvUse use, bool inInitFlow);

    /** \brief Says what frees a virtual environment that is in use, as a sentence for a dialog.
     *
     * A worker is freed by uninitializing the tool or switching to another one,
     * loaded modules only by a restart. The application name in the restart hint
     * is read from QCoreApplication.
     *
     * \param[in] use How the environment is used; not VirtualEnvUse::None.
     * \param[in] packageName The name of the package as the user knows it, e.g. "VoxTell".
     */
    MITKPYTHONSEGMENTATIONUI_EXPORT QString DescribeHowToFree(VirtualEnvUse use, const std::string& packageName);

    /** \brief The outcome of CheckVersionAndOfferUpdate(). */
    enum class VersionCheckOutcome
    {
      KeptInstalled, /**< \brief The installed version is supported, or the user keeps it. */
      Updated,       /**< \brief The package was updated in place. */
      Aborted        /**< \brief The installed version is not supported and was not updated, or the user cancelled. */
    };

    /** \brief Checks the installed version of a package during initialization
     *         and offers an in-place update if it is outdated.
     *
     * The installed version is compared against \p supportedVersions on every
     * call, so a package that predates this build is caught. Whether a newer
     * release exists is asked online at most once per application run and
     * distribution, so a user without network waits for the timeout only once.
     *
     * An update runs through QmitkPipInstallDialog in update mode. It is not
     * offered while the virtual environment is in use (see mitk::GetVirtualEnvUse());
     * the prompt says what frees it then.
     *
     * \param[in] parent Parent of the dialogs.
     * \param[in] probe A probe of the virtual environment. It is ended after the
     *            version check, since its worker would count as using the
     *            environment and block the update.
     * \param[in] packageName The name of the package as the user knows it, e.g. "VoxTell".
     * \param[in] distributionName The pip distribution to look up, e.g. "voxtell".
     * \param[in] supportedVersions The versions this build supports.
     * \param[in] upgradeSpec What to install for the update. Names the virtual environment.
     */
    MITKPYTHONSEGMENTATIONUI_EXPORT VersionCheckOutcome CheckVersionAndOfferUpdate(QWidget* parent, PackageProbe& probe, const std::string& packageName, const std::string& distributionName, const VersionRange& supportedVersions, const PipInstallSpec& upgradeSpec);

    /** \brief Reports that the initialization of a package failed.
     *
     * A Python error, from a PythonWorker or the embedded interpreter, carries
     * a traceback, and a worker that ended carries its last output. Both are
     * put into the details of the dialog behind a generic headline. Any other
     * error is a message for the user and is shown as it is. The error is
     * logged either way.
     *
     * \param[in] parent Parent of the dialog.
     * \param[in] packageName The name of the package as the user knows it, e.g. "VoxTell".
     * \param[in] e The error.
     */
    MITKPYTHONSEGMENTATIONUI_EXPORT void ShowInitializationError(QWidget* parent, const std::string& packageName, const Exception& e);
  }
}

#endif
