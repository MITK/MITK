/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkTorchInstall.h>

// torch 2.8, the last release with Pascal kernels (GeForce 10 series), keeps
// gigabytes per call on the CPU at the sizes nnInteractive uses, and it
// publishes no wheels for CPython 3.14. 2.9 has a blocking defect for us. So
// it is 2.10, whose CUDA builds start at Turing. Keep in sync with
// MinimumComputeCapability() in mitkTorchDevice.cpp.
//
// torchvision has to be pinned alongside torch and installed from the same
// index, because every torchvision release hard-pins one torch version; the
// torch upper bound is what keeps it on the matching minor. Tools do not need
// torchvision themselves, but their dependencies can pull it in (TotalSegmentator
// does through timm). Without the pin the resolve takes the latest torchvision
// and drags torch off the CUDA wheel onto a non-CUDA build from PyPI.
std::vector<std::string> mitk::Torch::Requirements()
{
  return { "torch>=2.10.0,<2.11.0", "torchvision>=0.25.0,<1.0.0" };
}

// cu128, the CUDA build PyPI ships for Linux, so every platform needs the same
// driver. All torch-based tools use the same index, so they all pull a build
// that runs on the GPUs we support.
std::string mitk::Torch::CudaIndexUrl()
{
#if defined(_WIN32)
  return "https://download.pytorch.org/whl/cu128";
#else
  return {};
#endif
}

mitk::PipInstallGroup mitk::Torch::BuildInstallGroup()
{
  PipInstallGroup group;
  group.requirements = Requirements();
  group.indexUrl = CudaIndexUrl();

  return group;
}
