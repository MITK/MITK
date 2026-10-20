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
    /** \brief Builds the pip install spec for a fresh TotalSegmentator install.
     *
     * Installs PyTorch (from the CUDA wheel index on Windows) and the
     * TotalSegmentator and nnU-Net releases pinned in mitkTotalSegmentatorTool.h.
     */
    MITKPYTHONSEGMENTATIONUI_EXPORT PipInstallSpec BuildInstallSpec(const std::string& venvName);
  }
}

#endif
