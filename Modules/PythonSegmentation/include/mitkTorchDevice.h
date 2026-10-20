/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkTorchDevice_h
#define mitkTorchDevice_h

#include <MitkPythonSegmentationExports.h>

#include <mitkPythonWorker.h>

#include <optional>
#include <string>

namespace mitk
{

  namespace Torch
  {
    /** \brief Specifies the computation backends available for PyTorch based tools.
     *
     * Backends define the computational resources used, such as GPU (CUDA) or
     * CPU for processing.
     */
    enum class Backend
    {
      CUDA, /**< CUDA backend for GPU computation (fast) */
      CPU   /**< Backend for CPU computation (slow) */
    };

    /** \brief Information about a CUDA-capable GPU device. */
    struct CUDADeviceInfo
    {
      int Index = 0;         /**< \brief Device index, as in "cuda:N". */
      std::string Name;      /**< \brief Device name (e.g., "NVIDIA GeForce RTX 3080"). */
      int Major = 0;         /**< \brief CUDA compute capability major version. */
      int Minor = 0;         /**< \brief CUDA compute capability minor version. */
      int TotalMemoryMB = 0; /**< \brief Total device memory in megabytes. */
    };

    /** \brief A CUDA compute capability, e.g. 6.1 for Pascal. */
    struct ComputeCapability
    {
      int Major;
      int Minor;
    };

    /** \brief Returns the lowest compute capability the installed PyTorch build has kernels for.
     *
     * Pascal (6.1) below CPython 3.14, Turing (7.5) from 3.14 on, following the
     * torch release the install requests.
     *
     * \sa mitk::Torch::Requirements()
     */
    MITKPYTHONSEGMENTATION_EXPORT ComputeCapability MinimumComputeCapability();

    /** \brief Extracts N from a torch device string of the form "cuda:N".
     *
     * \param[in] deviceString The device string, surrounding whitespace is ignored.
     *
     * \return The device index, or \c std::nullopt if the string does not name a
     *         CUDA device by its index.
     */
    MITKPYTHONSEGMENTATION_EXPORT std::optional<int> ParseCUDADeviceIndex(const std::string& deviceString);

    /** \brief Queries a CUDA device via PyTorch in a worker process.
     *
     * Imports torch in the worker and reads the properties of the device.
     *
     * \param[in] worker A worker of the virtual environment that has PyTorch
     *                   installed, running the module "torch_device.py" from the
     *                   resources of this module.
     * \param[in] deviceIndex The index of the CUDA device.
     * \param[in] progress Optional hook for the call, see PythonWorker::Call().
     *
     * \return The device information, or \c std::nullopt if PyTorch cannot be
     *         imported, CUDA is not available, or the device does not exist.
     *
     * \throw PythonWorkerExitedException if the worker is gone, and whatever
     *        \p progress throws.
     */
    MITKPYTHONSEGMENTATION_EXPORT std::optional<CUDADeviceInfo> QueryCUDADevice(PythonWorker& worker, int deviceIndex, const PythonWorker::ProgressHook& progress = {});

    /** \brief Which device the user wants a PyTorch based tool to compute on. */
    enum class BackendPreference
    {
      Auto, /**< The preferred GPU if it is suitable, the CPU otherwise. */
      CPU,  /**< The CPU, without looking at any GPU. */
      GPU   /**< The preferred GPU, even if it seems unsuitable. */
    };

    /** \brief Reads a backend preference as the preference pages store it.
     *
     * \param[in] value "auto", "cpu", or "gpu".
     *
     * \throw mitk::Exception if \p value is none of them.
     */
    MITKPYTHONSEGMENTATION_EXPORT BackendPreference ParseBackendPreference(const std::string& value);

    /** \brief The outcome of SelectDevice(). */
    struct DeviceSelection
    {
      Backend SelectedBackend = Backend::CPU;     /**< \brief The backend to compute on. */
      std::string Device = "cpu";                 /**< \brief The torch device string to use, e.g. "cuda:0" or "cpu". */
      std::optional<CUDADeviceInfo> CUDADevice;   /**< \brief The probed CUDA device, if one was found. */
    };

    /** \brief Decides which device PyTorch based inference runs on.
     *
     * With BackendPreference::CPU the CPU is selected without probing.
     * Otherwise the CUDA device named by \p gpuDevice is probed and used if it
     * meets MinimumComputeCapability() and \p minTotalMemoryMB. A device that
     * does not is replaced by the CPU for BackendPreference::Auto, while
     * BackendPreference::GPU uses it anyway. The decision is logged.
     *
     * \param[in] worker A worker as required by QueryCUDADevice().
     * \param[in] backend The device the user wants to compute on.
     * \param[in] gpuDevice The preferred GPU as a torch device string, e.g. "cuda:0".
     * \param[in] minTotalMemoryMB The least device memory, in megabytes, the workload needs.
     * \param[in] progress Optional hook for the probe, see PythonWorker::Call().
     *
     * \return The selected device.
     *
     * \throw See QueryCUDADevice().
     */
    MITKPYTHONSEGMENTATION_EXPORT DeviceSelection SelectDevice(PythonWorker& worker, BackendPreference backend, const std::string& gpuDevice, int minTotalMemoryMB, const PythonWorker::ProgressHook& progress = {});
  }
}

#endif
