/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkTotalSegmentatorInstall_h
#define mitkTotalSegmentatorInstall_h

#include <MitkPythonSegmentationUIExports.h>

#include <mitkPipPackageInfo.h>

#include <string>

namespace mitk
{
  namespace TotalSegmentator
  {
    /** \brief The exact TotalSegmentator release this MITK build installs and requires.
     *
     * Pinned exactly (not a range) because TotalSegmentator ships breaking changes
     * even in minor releases. Bumping it here means a new release cycle picks up the
     * newer version; existing environments can be updated in place.
     */
    inline constexpr const char* REQUIRED_VERSION = "2.14.0";

    /** \brief Name of the managed virtual environment created under the MITK venv base path. */
    inline constexpr const char* VENV_NAME = "TotalSegmentator";

    /** \brief Builds the pip install spec for a fresh TotalSegmentator install.
     *
     * Installs PyTorch (from the CUDA wheel index on Windows) and the pinned
     * TotalSegmentator release, then dumps the per-task label maps to a JSON file
     * inside the environment so the tool can name output labels without parsing the
     * package source.
     */
    MITKPYTHONSEGMENTATIONUI_EXPORT PipInstallSpec BuildInstallSpec(const std::string& venvName);
  }
}

#endif
