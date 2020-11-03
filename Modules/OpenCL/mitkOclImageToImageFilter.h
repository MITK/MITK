/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef __mitkOclImageToImageFilter_h
#define __mitkOclImageToImageFilter_h

#include "mitkOclImageFilter.h"

namespace mitk
{
class OclImageFilter;
class OclImageToImageFilter;

/** @class OclImageToImageFilter
  * @brief The OclImageToImageFilter is the base class for all OpenCL image filter generating images.
  */
class MITKOPENCL_EXPORT OclImageToImageFilter: public OclImageFilter
{
public:
  /*!
    * \brief Returns an mitk::Image::Pointer containing the filtered data.
    */
  mitk::Image::Pointer GetOutput();

  /*!
    * \brief Returns a pointer to the graphics memory.
    *
    * Use this method when executing two and more filters on the GPU for fast access.
    * This method does not copy the data to RAM. It returns only a pointer.
    */
  mitk::OclImage::Pointer GetGPUOutput();

protected:
  /**
   * @brief OclImageToImageFilter Default constructor.
   */
  OclImageToImageFilter();

  /** @brief Destructor */
  virtual ~OclImageToImageFilter();

  /** Output Image */
  mitk::OclImage::Pointer m_Output;

  /** @brief (Virtual) method Update() to be implemented in derived classes. */
  virtual void Update() = 0;

  /** @brief (Virtual) method returning the format in which the output image will be returned */
  virtual mitk::PixelType GetOutputType();

  /**
   * @brief InitExec Initialize the execution
   * @param ckKernel The GPU kernel.
   * @throws mitk::Exception if something goes wrong.
   * @return True for success.
   */
  bool InitExec(cl_kernel ckKernel);
  bool InitExec(cl_kernel ckKernel, unsigned int* dimensions);

  /** @brief Get the memory size needed for each element */
  virtual int GetBytesPerElem();

private:
  /** Block dimensions */
  unsigned int m_BlockDims[3];

};
}
#endif // __mitkOclImageToImageFilter_h
