/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkTorchInstall.h>

#include <mitkPythonHelper.h>

// torch 2.8 is the last release whose CUDA builds contain Pascal kernels
// (sm_61, GeForce 10 series), and it publishes no wheels for CPython 3.14 or
// newer. There the oldest usable release is 2.10 (2.9 has a blocking defect for
// us), which no longer runs on Pascal, so the minimum GPU rises to Turing.
// Keep in sync with MinimumComputeCapability() in mitkTorchDevice.cpp.
//
// torchvision has to be pinned alongside torch and installed from the same
// index, because every torchvision release hard-pins one torch version; the
// torch upper bound is what keeps it on the matching minor. Tools do not need
// torchvision themselves, but their dependencies can pull it in (TotalSegmentator
// does through timm). Without the pin the resolve takes the latest torchvision
// and drags torch off the CUDA wheel onto a non-CUDA build from PyPI.
std::vector<std::string> mitk::Torch::Requirements()
{
  if constexpr (mitk::PythonHelper::VERSION_MINOR >= 14)
    return { "torch>=2.10.0,<2.11.0", "torchvision>=0.25.0,<1.0.0" };
  else
    return { "torch>=2.8.0,<2.9.0", "torchvision>=0.23.0,<1.0.0" };
}

// cu128: with CUDA 12.9 our lowest supported GPU architecture hits "no kernel
// image is available". All torch-based tools use the same index, so they all
// pull a build that runs on the GPUs we support.
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
