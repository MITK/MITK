/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitknnInteractiveUpdatePrompt_h
#define mitknnInteractiveUpdatePrompt_h

#include <MitkPythonSegmentationUIExports.h>

#include <mitknnInteractiveVersion.h>

class QWidget;

namespace mitk
{
  namespace nnInteractive
  {
    /** \brief The user's response to the version-update prompt. */
    enum class UpdatePromptChoice
    {
      Update,            /**< \brief Run the in-place update now. */
      ContinueInstalled, /**< \brief Keep using the installed version. */
      Cancel             /**< \brief Do nothing / abort the surrounding flow. */
    };

    /** \brief Shows the standard nnInteractive version-status dialog and returns
     *         the user's choice.
     *
     * Handles the BelowMinimum and UpdateAvailable verdicts (read from
     * \p result.Status); passing any other status is a usage error. The wording
     * and the available buttons adapt to two flags:
     *
     * - \p modulesLoaded: nnInteractive is already imported into this process, so
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
     * \param[in] result Version check verdict; only BelowMinimum / UpdateAvailable.
     * \param[in] modulesLoaded Whether nnInteractive modules are loaded in-process.
     * \param[in] inInitFlow Whether the prompt is shown during initialization.
     *
     * \return The user's choice. BelowMinimum never yields ContinueInstalled.
     */
    MITKPYTHONSEGMENTATIONUI_EXPORT UpdatePromptChoice ShowUpdatePrompt(QWidget* parent, const VersionCheckResult& result, bool modulesLoaded, bool inInitFlow);
  }
}

#endif
