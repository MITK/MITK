/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitknnInteractiveInstall_h
#define mitknnInteractiveInstall_h

#include <MitkPythonSegmentationUIExports.h>

#include <mitkPipPackageInfo.h>

#include <string>

namespace mitk
{
  class IPreferences;

  namespace nnInteractive
  {
    /** \brief Builds the pip install spec for a fresh nnInteractive install.
     *
     * Full mode installs PyTorch and nnInteractive and queues a post-install step
     * that pre-downloads the model checkpoint (unless a local checkpoint folder is
     * configured); client-only mode installs just the lightweight, torch-free
     * nninteractive-client. On Windows the torch group uses the CUDA wheel index.
     *
     * \param[in] prefs Preferences used to resolve the model source and checkpoint
     *                  for the pre-download step (may be \c nullptr to skip it).
     * \param[in] venvName Virtual environment to create and install into.
     * \param[in] clientOnly Whether to build a client-only (remote) install.
     */
    MITKPYTHONSEGMENTATIONUI_EXPORT PipInstallSpec BuildInstallSpec(IPreferences* prefs, const std::string& venvName, bool clientOnly);

    /** \brief Builds the pip spec for an in-place update of an existing install.
     *
     * Reuses the resolve-then-install engine with \c --upgrade; the venv already
     * exists, so pip is not upgraded first. On Windows the torch group is upgraded
     * from the CUDA wheel index so a transitive torch bump cannot pull a non-CUDA
     * wheel from PyPI.
     *
     * \param[in] venvName Existing virtual environment to upgrade in place.
     * \param[in] clientOnly Whether this upgrades nninteractive-client (vs nnInteractive).
     */
    MITKPYTHONSEGMENTATIONUI_EXPORT PipInstallSpec BuildUpgradeSpec(const std::string& venvName, bool clientOnly);
  }
}

#endif
