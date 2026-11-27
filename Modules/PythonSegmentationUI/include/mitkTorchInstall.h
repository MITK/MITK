/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkTorchInstall_h
#define mitkTorchInstall_h

#include <MitkPythonSegmentationUIExports.h>

#include <mitkPipPackageInfo.h>

#include <string>
#include <vector>

namespace mitk::Torch
{
  /** \brief Returns the torch and torchvision requirements of the torch-based tools.
   *
   * torch 2.10. The requirements carry upper bounds so torchvision stays on the
   * minor that matches torch.
   *
   * \sa MinimumComputeCapability()
   */
  MITKPYTHONSEGMENTATIONUI_EXPORT std::vector<std::string> Requirements();

  /** \brief Returns the pip index URL that provides CUDA builds of torch.
   *
   * \return The CUDA wheel index on Windows, an empty string elsewhere, where
   *         the default PyPI index already provides the right builds.
   */
  MITKPYTHONSEGMENTATIONUI_EXPORT std::string CudaIndexUrl();

  /** \brief Builds the install group that every torch-based tool installs first.
   *
   * Combines Requirements() with CudaIndexUrl(). Install it before the group
   * of the tool itself, so the tool's own dependencies resolve against the
   * torch that is already present instead of pulling a different one.
   */
  MITKPYTHONSEGMENTATIONUI_EXPORT PipInstallGroup BuildInstallGroup();
}

#endif
