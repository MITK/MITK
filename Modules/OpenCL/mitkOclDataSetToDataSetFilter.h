/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef __mitkOclDataSetToDataSetFilter_h
#define __mitkOclDataSetToDataSetFilter_h

#include "mitkOclDataSetFilter.h"

namespace mitk
{
class OclDataSetFilter;
class OclDataSetToDataSetFilter;

/** @class OclDataSetToDataSetFilter
  * @brief The OclDataSetToDataSetFilter is the base class for all OpenCL DataSet filter generating DataSets.
  */
class MITKOPENCL_EXPORT OclDataSetToDataSetFilter: public OclDataSetFilter
{
public:
  /*!
    * \brief Returns an pointer to the filtered data.
    */
  void* GetOutput();

  /*!
    * \brief Returns a pointer to the graphics memory.
    *
    * Use this method when executing two and more filters on the GPU for fast access.
    * This method does not copy the data to RAM. It returns only a pointer.
    */
  mitk::OclDataSet::Pointer GetGPUOutput();

protected:
  /**
   * @brief OclDataSetToDataSetFilter Default constructor.
   */
  OclDataSetToDataSetFilter();

  /** @brief Destructor */
  virtual ~OclDataSetToDataSetFilter();

  /** Output DataSet */
  mitk::OclDataSet::Pointer m_Output;

  /** @brief (Virtual) method Update() to be implemented in derived classes. */
  virtual void Update() = 0;

  /**
   * @brief InitExec Initialize the execution
   * @param ckKernel The GPU kernel.
   * @param dimensions
   * @param outputDataSize
   * @param outputBpE
   * @throws mitk::Exception if something goes wrong.
   * @return True for success.
   */
  bool InitExec(cl_kernel ckKernel, unsigned int* dimensions, size_t outputDataSize, unsigned int outputBpE);

  bool InitExecNoInput(cl_kernel ckKernel, unsigned int* dimensions, size_t outputDataSize, unsigned int outputBpE);

  /** @brief Get the memory size needed for each element */
  virtual int GetBytesPerElem();

  unsigned int m_CurrentSizeOutput;

private:
  /** Block dimensions */
  unsigned int m_BlockDims[3];
};
}
#endif // __mitkOclDataSetToDataSetFilter_h
