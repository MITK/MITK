/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkTorchDevice.h>

#include <mitkExceptionMacro.h>
#include <mitkLog.h>
#include <mitkMemoryUtilities.h>
#include <mitkPythonHelper.h>
#include <mitkPythonWorker.h>

#include <charconv>
#include <cstdint>
#include <regex>

namespace
{
  // Device memory is given in megabytes of 1024 * 1024 bytes, as torch reports it.
  std::uint64_t MegabytesToBytes(int megabytes)
  {
    return static_cast<std::uint64_t>(megabytes) * 1024 * 1024;
  }
}

// The architectures follow the PyTorch build the install requests: torch 2.8
// still runs on Pascal, while the 2.10 required from CPython 3.14 on starts at
// Turing. Keep in sync with Requirements() in mitkTorchInstall.cpp.
mitk::Torch::ComputeCapability mitk::Torch::MinimumComputeCapability()
{
  if constexpr (PythonHelper::VERSION_MINOR >= 14)
    return { 7, 5 };
  else
    return { 6, 1 };
}

std::optional<int> mitk::Torch::ParseCUDADeviceIndex(const std::string& deviceString)
{
  const std::regex regex(R"(^\s*cuda:(\d+)\s*$)");
  std::smatch match;

  if (!std::regex_match(deviceString, match, regex))
    return std::nullopt;

  const auto digits = match[1].str();
  int index = 0;

  // The digits can be more than an int holds; that names no device either.
  if (std::from_chars(digits.data(), digits.data() + digits.size(), index).ec != std::errc())
    return std::nullopt;

  return index;
}

std::optional<mitk::Torch::CUDADeviceInfo> mitk::Torch::QueryCUDADevice(PythonWorker& worker, int deviceIndex, const PythonWorker::ProgressHook& progress)
{
  try
  {
    const auto result = worker.Call("torch_device.query_cuda_device", { { "index", deviceIndex } }, {}, progress);

    if (result.is_null())
      return std::nullopt;

    CUDADeviceInfo info;
    info.Index = deviceIndex;
    info.Name = result.value("name", std::string());
    info.TotalMemoryMB = result.value("total_memory_mb", 0);
    info.Major = result.value("major", 0);
    info.Minor = result.value("minor", 0);

    return info;
  }
  catch (const PythonWorkerError& e)
  {
    // Only a failure in Python means there is no usable device. A worker that
    // is gone or a cancelled call has to reach the caller.
    MITK_WARN << "Could not query CUDA device " << deviceIndex << ": " << e.GetDescription();
    return std::nullopt;
  }
}

mitk::Torch::BackendPreference mitk::Torch::ParseBackendPreference(const std::string& value)
{
  if (value == "auto")
    return BackendPreference::Auto;

  if (value == "cpu")
    return BackendPreference::CPU;

  if (value == "gpu")
    return BackendPreference::GPU;

  mitkThrow() << "Unknown backend preference \"" << value << "\". Expected \"auto\", \"cpu\", or \"gpu\".";
}

mitk::Torch::DeviceSelection mitk::Torch::SelectDevice(PythonWorker& worker, BackendPreference backend, const std::string& gpuDevice, int minTotalMemoryMB, const PythonWorker::ProgressHook& progress)
{
  DeviceSelection selection;

  if (backend == BackendPreference::CPU)
    return selection;

  bool useCUDADevice = false;

  if (const auto deviceIndex = ParseCUDADeviceIndex(gpuDevice); deviceIndex.has_value())
  {
    selection.CUDADevice = QueryCUDADevice(worker, deviceIndex.value(), progress);
  }

  if (selection.CUDADevice.has_value())
  {
    const auto& deviceInfo = selection.CUDADevice.value();

    MITK_INFO << "Found CUDA device: " << deviceInfo.Name;
    MITK_INFO << "  Compute capability: " << deviceInfo.Major << "." << deviceInfo.Minor;
    MITK_INFO << "  Total memory: " << MemoryUtilities::FormatSize(MegabytesToBytes(deviceInfo.TotalMemoryMB));

    useCUDADevice = true;

    const auto minimum = MinimumComputeCapability();

    if (deviceInfo.Major < minimum.Major ||
        (deviceInfo.Major == minimum.Major && deviceInfo.Minor < minimum.Minor))
    {
      MITK_WARN << "Minimum required compute capability is " << minimum.Major << '.' << minimum.Minor;
      useCUDADevice = false;
    }

    if (deviceInfo.TotalMemoryMB < minTotalMemoryMB)
    {
      MITK_WARN << "Minimum required total memory is " << MemoryUtilities::FormatSize(MegabytesToBytes(minTotalMemoryMB));
      useCUDADevice = false;
    }
  }

  if (!useCUDADevice)
  {
    if (backend == BackendPreference::Auto)
    {
      MITK_WARN << "No compatible CUDA device detected. Falling back to CPU processing.";
    }
    else
    {
      MITK_WARN << "CPU backend would have been auto-selected, but the CUDA backend has been manually enforced. "
                << "Continue at your own risk.";
      useCUDADevice = true;
    }
  }

  if (useCUDADevice)
  {
    selection.SelectedBackend = Backend::CUDA;
    selection.Device = gpuDevice;
  }

  return selection;
}
