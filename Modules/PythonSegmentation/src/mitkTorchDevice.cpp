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
#include <mitkPythonContext.h>
#include <mitkPythonHelper.h>

#include <regex>
#include <sstream>

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

  if (std::regex_match(deviceString, match, regex))
    return std::stoi(match[1].str());

  return std::nullopt;
}

std::optional<mitk::Torch::CUDADeviceInfo> mitk::Torch::QueryCUDADevice(PythonContext& context, int deviceIndex)
{
  std::ostringstream pyCommands; pyCommands
    << "import torch\n"
    << "mitk_cuda_found = False\n"
    << "try:\n"
    << "    if torch.cuda.is_available():\n"
    << "        mitk_cuda_name = torch.cuda.get_device_name(" << deviceIndex << ")\n"
    << "        mitk_cuda_props = torch.cuda.get_device_properties(" << deviceIndex << ")\n"
    << "        mitk_cuda_memory_mb = mitk_cuda_props.total_memory // (1024 ** 2)\n"
    << "        mitk_cuda_major = mitk_cuda_props.major\n"
    << "        mitk_cuda_minor = mitk_cuda_props.minor\n"
    << "        mitk_cuda_found = True\n"
    << "except Exception:\n"
    << "    pass\n";

  try
  {
    context.Execute(pyCommands.str());

    if (!context.GetVariableAsBool("mitk_cuda_found").value_or(false))
      return std::nullopt;

    CUDADeviceInfo info;
    info.Index = deviceIndex;
    info.Name = context.GetVariableAsString("mitk_cuda_name").value_or("");
    info.TotalMemoryMB = context.GetVariableAsInt("mitk_cuda_memory_mb").value_or(0);
    info.Major = context.GetVariableAsInt("mitk_cuda_major").value_or(0);
    info.Minor = context.GetVariableAsInt("mitk_cuda_minor").value_or(0);

    return info;
  }
  catch (...)
  {
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

mitk::Torch::DeviceSelection mitk::Torch::SelectDevice(PythonContext& context, BackendPreference backend, const std::string& gpuDevice, int minTotalMemoryMB)
{
  DeviceSelection selection;

  if (backend == BackendPreference::CPU)
    return selection;

  bool useCUDADevice = false;

  if (const auto deviceIndex = ParseCUDADeviceIndex(gpuDevice); deviceIndex.has_value())
  {
    selection.CUDADevice = QueryCUDADevice(context, deviceIndex.value());
  }

  if (selection.CUDADevice.has_value())
  {
    const auto& deviceInfo = selection.CUDADevice.value();

    MITK_INFO << "Found CUDA device: " << deviceInfo.Name;
    MITK_INFO << "  Compute capability: " << deviceInfo.Major << "." << deviceInfo.Minor;
    MITK_INFO << "  Total memory: " << deviceInfo.TotalMemoryMB << " MB";

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
      MITK_WARN << "Minimum required total memory is " << minTotalMemoryMB / 1000 << " GB";
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
