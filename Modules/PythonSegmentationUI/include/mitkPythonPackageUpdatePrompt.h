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

#include <string>

class QWidget;

namespace mitk
{
  class Exception;
  class PythonContext;

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
     * and the available buttons adapt to two flags:
     *
     * - \p modulesLoaded: the package is already imported into this process, so
     *   an in-place update would fail on Windows (locked files). The dialog then
     *   offers no "Update now" and asks the user to restart first.
     * - \p inInitFlow: shown during initialization, where declining still proceeds
     *   with the working installed version (offers "Continue with installed", which
     *   maps to ContinueInstalled). When \c false (the preferences page), declining
     *   simply closes the dialog.
     *
     * The application name used in the restart hint is read from QCoreApplication.
     *
     * \param[in] parent Dialog parent (may be \c nullptr).
     * \param[in] packageName The name of the package as the user knows it, e.g. "nnInteractive".
     * \param[in] supportedVersions The versions this MITK build supports.
     * \param[in] result Version check verdict; only BelowMinimum / UpdateAvailable.
     * \param[in] modulesLoaded Whether modules of the package are loaded in-process.
     * \param[in] inInitFlow Whether the prompt is shown during initialization.
     *
     * \return The user's choice. BelowMinimum never yields ContinueInstalled.
     */
    MITKPYTHONSEGMENTATIONUI_EXPORT UpdatePromptChoice ShowUpdatePrompt(QWidget* parent, const std::string& packageName, const VersionRange& supportedVersions, const VersionCheckResult& result, bool modulesLoaded, bool inInitFlow);

    /** \brief The outcome of CheckVersionAndOfferUpdate(). */
    enum class VersionCheckOutcome
    {
      KeptInstalled, /**< \brief The installed version is supported, or the user keeps it. */
      Updated,       /**< \brief The package was updated in place. A Python context has to be recreated to see it. */
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
     * offered while modules of the virtual environment are loaded, as pip
     * cannot replace mapped files on Windows; the prompt asks to restart then.
     *
     * \param[in] parent Parent of the dialogs.
     * \param[in] context An activated Python context of the virtual environment.
     * \param[in] packageName The name of the package as the user knows it, e.g. "VoxTell".
     * \param[in] distributionName The pip distribution to look up, e.g. "voxtell".
     * \param[in] supportedVersions The versions this build supports.
     * \param[in] upgradeSpec What to install for the update. Names the virtual environment.
     */
    MITKPYTHONSEGMENTATIONUI_EXPORT VersionCheckOutcome CheckVersionAndOfferUpdate(QWidget* parent, PythonContext& context, const std::string& packageName, const std::string& distributionName, const VersionRange& supportedVersions, const PipInstallSpec& upgradeSpec);

    /** \brief Reports that the initialization of a package failed.
     *
     * An error from the embedded Python interpreter carries a traceback, which
     * is put into the details of the dialog behind a generic headline. Any
     * other error is a message for the user and is shown as it is. The error
     * is logged either way.
     *
     * \param[in] parent Parent of the dialog.
     * \param[in] packageName The name of the package as the user knows it, e.g. "VoxTell".
     * \param[in] e The error.
     */
    MITKPYTHONSEGMENTATIONUI_EXPORT void ShowInitializationError(QWidget* parent, const std::string& packageName, const Exception& e);
  }
}

#endif
