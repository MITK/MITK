/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkOclDataSet_h
#define mitkOclDataSet_h

#define GPU_DATA 0
#define CPU_DATA 1

#include <mitkImage.h>
#include "MitkOpenCLExports.h"

#include "mitkOclBaseData.h"
#include "mitkOpenCLActivator.h"

#include <mitkException.h>

namespace mitk {

/*!
 * \brief Class implementing processing of arbitrary data sets for GPU Image Processing
  *
  *  The class holds a pointer to the data stored in RAM and performs an
  *  on-demand-copy to the graphics memory. It is the basic data structure for all
  *  mitk::oclDataSetToDataSetFilter classes
  */
class MITKOPENCL_EXPORT OclDataSet : public OclBaseData
{
public:
  mitkClassMacro(OclDataSet, OclBaseData);
  itkFactorylessNewMacro(Self);
  itkCloneMacro(Self);

  /*!  \brief Copies the RAM-stored data to GMEM */
  virtual int TransferDataToGPU(cl_command_queue);

  /*! \brief Copies the in GMEM stored data to RAM */
  virtual void* TransferDataToCPU(cl_command_queue);

  /*! \brief Returns the pointer to the referenced data */
  void* GetData()
  {
    return m_Data;
  }

  /** Returns the pointer to the GPU buffer */
  cl_mem GetGPUBuffer();

  /** Create the GPU buffer for image
      *
      */
  cl_mem CreateGPUBuffer();

  /** \brief Returns the status of the image buffer
     *
     * @param _type The flag to specify the buffer type ( GPU / CPU )
     */
  bool IsModified(int _type);

  using OclBaseData::Modified;

  /** \brief Set the modified flag for one of the buffers
     *
     * @param _type The flag to specify the buffer type ( GPU / CPU )
     */
  void Modified(int _type);

  /** \brief Initialze the OclDataSet with data. */
  void SetData(void* data)
  {
    this->m_cpuModified = true;
    this->m_gpuModified = false;
    m_Data = data;
  }

  /*! \brief returns the amount of elements in the DataSet */
  size_t GetBufferSize() const
  {
    return this->m_bufferSize;
  }

  short GetBytesPerElement() const
  {
    return this->m_BpE;
  }

  /** @brief Set the amount of elements in buffer*/
  void SetBufferSize(size_t size);

  /** @brief Set the DataSet memory Size per Element in Bytes*/
  void SetBpE(unsigned short BpE);

protected:
  /*! \brief Constructor */
  OclDataSet();

  /** @brief Destructor */
  virtual ~OclDataSet();

  /*! GMEM Image buffer */
  cl_mem m_gpuBuffer;

  /*! GPU Context the Buffer was created in, needed for access */
  cl_context m_context;

private:
  /*! GMEM Buffer Size in elements*/
  size_t m_bufferSize;

  bool m_gpuModified;
  bool m_cpuModified;

  /*! Reference to the data */
  void* m_Data;

  /*! Bytes per Element in Buffer*/
  unsigned short m_BpE;
};

}
#endif
