/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef __mitkOclBaseData_h
#define __mitkOclBaseData_h

#include "MitkOclExports.h"
#include "mitkOclUtils.h"

#include "mitkOpenCL.h"

#include <mitkCommon.h>
#include <itkObjectFactory.h>
#include <itkObject.h>

namespace mitk
{

  class MitkOcl_EXPORT OclBaseData : public itk::Object
  {
  public:
    mitkClassMacro(OclBaseData, itk::Object);

    /*!  \brief Copies the RAM-stored data to GPU-MEM.
     * This method has to be implemented for each data type.
     */
    virtual int TransferDataToGPU(cl_command_queue) = 0;

    /*! \brief Copies the in GPU-MEM stored data to RAM
     * This method has to be implemented for each data type.
     */
    virtual void* TransferDataToCPU(cl_command_queue) = 0;

    /** \brief Set the modified flag for one of the GPU buffer */
    void GPUModified();

    /** \brief Set the modified flag for one of the CPU buffer */
    void CPUModified();


    /** \brief Get the pointer to the buffer on the device (GPU) */
    virtual cl_mem GetGPUBuffer();

  protected:
    /** \brief Default constructor. */
    OclBaseData();

    /** \brief Default destructor. */
    virtual ~OclBaseData();

  private:

    /** \brief Size of the data buffer. CPU size equals GPU size. */
    size_t m_BufferSize;

    /** \brief Pointer to the buffer on the device (GPU) */
    cl_mem m_GPUBuffer;

    /** \brief Pointer to the buffer on the host (CPU) */
    void* m_CPUBuffer;
  };

}

#endif // __mitkOclBaseData_h
