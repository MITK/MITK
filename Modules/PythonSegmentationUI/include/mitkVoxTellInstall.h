/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkVoxTellInstall_h
#define mitkVoxTellInstall_h

#include <MitkPythonSegmentationUIExports.h>

#include <mitkPipPackageInfo.h>
#include <mitkVoxTellTool.h>

#include <string>
#include <vector>

namespace mitk
{
  class IPreferences;

  namespace VoxTell
  {
    /** \brief Builds the pip install spec for a fresh VoxTell install.
     *
     * Installs PyTorch (from the CUDA wheel index on Windows) and VoxTell, and
     * then downloads what VoxTell loads from the Hugging Face Hub: the
     * embeddings of the prompts it knows, the model unless a local model folder
     * is configured, and the text encoder if asked for. Without these steps the
     * first initialization or the first unknown prompt would surprise the user
     * with a download of gigabytes. They are optional, since whatever is missing
     * is downloaded again when it is needed.
     *
     * \param[in] prefs Preferences used to resolve the model source. If
     *                  \c nullptr, the model is downloaded.
     * \param[in] venvName Virtual environment to create and install into.
     * \param[in] includeTextModel Whether to download the text encoder, which is
     *                  only needed for prompts that VoxTell does not know.
     */
    MITKPYTHONSEGMENTATIONUI_EXPORT PipInstallSpec BuildInstallSpec(IPreferences* prefs, const std::string& venvName, bool includeTextModel);

    /** \brief Builds the pip spec for an in-place update of an existing install.
     *
     * Reuses the resolve-then-install engine with \c --upgrade; the venv already
     * exists, so pip is not upgraded first. On Windows the torch group is
     * upgraded from the CUDA wheel index so a transitive torch bump cannot pull
     * a non-CUDA wheel from PyPI.
     *
     * \param[in] venvName Existing virtual environment to upgrade in place.
     */
    MITKPYTHONSEGMENTATIONUI_EXPORT PipInstallSpec BuildUpgradeSpec(const std::string& venvName);

    /** \brief Builds a step that downloads files of the Hugging Face Hub into its cache.
     *
     * The step reports its progress in bytes and does nothing for files that
     * are cached already. Without network access, files that are cached
     * already count as downloaded.
     *
     * \param[in] displayName What the user sees while the step runs.
     * \param[in] files The files to download.
     *
     * \sa QmitkVenvProcess::RunStep()
     */
    MITKPYTHONSEGMENTATIONUI_EXPORT PostInstallStep BuildDownloadStep(const std::string& displayName, const std::vector<RepoFiles>& files);
  }
}

#endif
