/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkOclDataSet.h"
#include "mitkCommon.h"
#include "mitkLogMacros.h"

#include "mitkOclUtils.h"

#include <fstream>

//#define SHOW_MEM_INFO

mitk::OclDataSet::OclDataSet() : m_gpuBuffer(nullptr), m_context(nullptr), m_bufferSize(0), m_gpuModified(false), m_cpuModified(false),
  m_Data(nullptr), m_BpE(1)
{
}

mitk::OclDataSet::~OclDataSet()
{
  MITK_DEBUG << "OclDataSet Destructor";

  //release GMEM Image buffer
  if (m_gpuBuffer) clReleaseMemObject(m_gpuBuffer);
}


cl_mem mitk::OclDataSet::CreateGPUBuffer()
{
  MITK_DEBUG << "InitializeGPUBuffer call with: BPE=" << m_BpE;

  us::ServiceReference<OclResourceService> ref = GetModuleContext()->GetServiceReference<OclResourceService>();
  OclResourceService* resources = GetModuleContext()->GetService<OclResourceService>(ref);

  m_context = resources->GetContext();

  int clErr;
  if (m_gpuBuffer) clReleaseMemObject(m_gpuBuffer);

  m_gpuBuffer = clCreateBuffer(m_context, CL_MEM_READ_WRITE, m_bufferSize * (size_t)m_BpE, nullptr, &clErr);

  #ifdef SHOW_MEM_INFO
  MITK_INFO << "Created GPU Buffer Object of size: " << (size_t)m_BpE * m_bufferSize << " Bytes";
  #endif

  CHECK_OCL_ERR(clErr);

  if (clErr != CL_SUCCESS)
    mitkThrow() << "openCL Error when creating Buffer";

  return m_gpuBuffer;
}

bool mitk::OclDataSet::IsModified(int _type)
{
  if (_type) return m_cpuModified;
  else return m_gpuModified;
}

void mitk::OclDataSet::Modified(int _type)
{
  // defines... GPU: 0, CPU: 1
  m_cpuModified = _type;
  m_gpuModified = !_type;
}

int mitk::OclDataSet::TransferDataToGPU(cl_command_queue gpuComQueue)
{
  cl_int clErr = 0;

  // check whether an image present
  if (m_Data == nullptr){
    MITK_ERROR("ocl.DataSet") << "(mitk) No data present!\n";
    return -1;
  }

  // there is a need for copy only if RAM-Data newer then GMEM data
  if (m_cpuModified)
  {
    //check the buffer
    if(m_gpuBuffer == nullptr)
    {
      CreateGPUBuffer();
    }

    if (m_gpuBuffer != nullptr)
    {
      clErr = clEnqueueWriteBuffer(gpuComQueue, m_gpuBuffer, CL_TRUE, 0, m_bufferSize * (size_t)m_BpE, m_Data, 0, NULL, NULL);
      MITK_DEBUG << "Wrote Data to GPU Buffer Object.";

      CHECK_OCL_ERR(clErr);
      m_gpuModified = true;

      if (clErr != CL_SUCCESS)
        mitkThrow() << "openCL Error when writing Buffer";
    }
    else
    {
      MITK_ERROR << "No GPU buffer present!";
    }
  }

  return clErr;
}

cl_mem mitk::OclDataSet::GetGPUBuffer()
{
  // query image object info only if already initialized
  if( this->m_gpuBuffer )
  {
    #ifdef SHOW_MEM_INFO
    cl_int clErr = 0;
    // clGetMemObjectInfo()
    cl_mem_object_type memInfo;
    clErr = clGetMemObjectInfo(this->m_gpuBuffer, CL_MEM_TYPE, sizeof(cl_mem_object_type), &memInfo, nullptr );
    CHECK_OCL_ERR(clErr);
    MITK_DEBUG << "Querying info for object, recieving: " << memInfo;
    #endif
  }

  return m_gpuBuffer;
}

void* mitk::OclDataSet::TransferDataToCPU(cl_command_queue gpuComQueue)
{
  cl_int clErr = 0;

  // if image created on GPU, needs to create mitk::Image
  if( m_gpuBuffer == nullptr ){
    MITK_ERROR("ocl.DataSet") << "(mitk) No buffer present!\n";
    return nullptr;
  }

  // check buffersize
  char* data = new char[m_bufferSize * (size_t)m_BpE];

  // debug info
  #ifdef SHOW_MEM_INFO
  oclPrintMemObjectInfo( m_gpuBuffer );
  #endif

  clErr = clEnqueueReadBuffer( gpuComQueue, m_gpuBuffer, CL_TRUE, 0, m_bufferSize * (size_t)m_BpE, data ,0, nullptr, nullptr);
  CHECK_OCL_ERR(clErr);

  if(clErr != CL_SUCCESS)
    mitkThrow() << "openCL Error when reading Output Buffer";

  clFlush( gpuComQueue );
  // the cpu data is same as gpu
  this->m_gpuModified = false;

  return (void*) data;
}

void mitk::OclDataSet::SetBufferSize(size_t size)
{
  m_bufferSize = size;
}

void mitk::OclDataSet::SetBpE(unsigned short BpE)
{
  m_BpE = BpE;
}
