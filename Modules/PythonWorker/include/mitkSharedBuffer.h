/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkSharedBuffer_h
#define mitkSharedBuffer_h

#include <mitkImage.h>

#include <nlohmann/json_fwd.hpp>

#include <MitkPythonWorkerExports.h>

#include <memory>
#include <vector>

namespace mitk
{
  class PythonWorker;

  /**
   * \brief Shared memory holding one array that a PythonWorker maps as a NumPy array.
   *
   * MITK creates every buffer and passes it as an argument of PythonWorker::Call().
   * The worker attaches to the buffer by name while it serves that call, and a
   * buffer is passed to one call only. On Linux and macOS the name is removed after
   * the call, so no other process can attach any more. On Windows the name lives as
   * long as the memory is in use, since the system removes a named mapping with
   * its last handle. Either way, a crash of the worker cannot leave the memory behind. The
   * memory itself stays valid in both processes as long as each keeps it mapped:
   * a tool may keep the array of a buffer across calls, and MITK keeps it until
   * the buffer is destroyed.
   *
   * New buffers are zero-filled. Shapes are in NumPy order, i.e. (z, y, x) for a
   * 3D image. Only single-component pixel types are supported.
   */
  class MITKPYTHONWORKER_EXPORT SharedBuffer
  {
  public:
    SharedBuffer(const PixelType& pixelType, const std::vector<std::size_t>& shape);
    ~SharedBuffer();

    SharedBuffer(const SharedBuffer&) = delete;
    SharedBuffer& operator=(const SharedBuffer&) = delete;

    /** \brief Copies one time step of an image into a new buffer. */
    static std::unique_ptr<SharedBuffer> CopyOf(const Image* image, TimeStepType timeStep = 0);

    /**
     * \brief Creates an image with the given geometry that uses the memory of this buffer
     * without copying it.
     *
     * The geometry must have one time step whose extent matches the shape. The image
     * keeps the memory mapped, so it may outlive the buffer.
     */
    Image::Pointer AsImage(const TimeGeometry& geometry) const;

    void* GetData() const;

    /** \brief Size of the array in bytes. */
    std::size_t GetSize() const;

    const PixelType& GetPixelType() const;
    const std::vector<std::size_t>& GetShape() const;

  private:
    friend class PythonWorker;

    /** \brief What a worker needs to map the buffer: name, NumPy dtype and shape. */
    nlohmann::json GetDescriptor() const;

    /** \brief Withdraws the name of the buffer. Existing mappings stay valid. */
    void Unpublish();

    bool IsPublished() const;

    struct Impl;
    std::shared_ptr<Impl> m_Impl;
  };
}

#endif
