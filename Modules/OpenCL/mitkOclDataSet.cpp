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

#include "mitkOclDataSet.h"
#include "mitkCommon.h"
#include "mitkLogMacros.h"

#include "mitkOclUtils.h"

#include <fstream>

mitk::OclDataSet::OclDataSet() : m_gpuBuffer(nullptr), m_context(nullptr), m_bufferSize(0), m_gpuModified(false), m_cpuModified(false),
  m_Data(nullptr), m_size(0), m_BpE(1)
{
}

mitk::OclDataSet::~OclDataSet()
{
  MITK_INFO << "OclDataSet Destructor";

  //release GMEM Image buffer
  if (m_gpuBuffer) clReleaseMemObject(m_gpuBuffer);
}


cl_mem mitk::OclDataSet::CreateGPUBuffer(unsigned int _size, unsigned int _bpe)
{
  MITK_INFO << "InitializeGPUBuffer call with: BPE=" << _bpe;

  m_bufferSize = _size;

  m_BpE = _bpe;

  us::ServiceReference<OclResourceService> ref = GetModuleContext()->GetServiceReference<OclResourceService>();
  OclResourceService* resources = GetModuleContext()->GetService<OclResourceService>(ref);

  m_context = resources->GetContext();

  int clErr;
  m_gpuBuffer = clCreateBuffer(m_context, CL_MEM_READ_ONLY, m_bufferSize * m_BpE, nullptr, &clErr);

  CHECK_OCL_ERR(clErr);

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
      MITK_ERROR("ocl.DataSet") << "(mitk) No GPU buffer present!\n";
      return -1;
    }

    if (m_gpuBuffer != nullptr)
    {
      clErr = clEnqueueWriteBuffer(gpuComQueue, m_gpuBuffer, CL_TRUE, 0, m_bufferSize * m_BpE, m_Data, 0, NULL, NULL);
    }
    else
    {
      MITK_ERROR << "No GPU buffer present!";
    }
    CHECK_OCL_ERR(clErr);
    m_gpuModified = true;
  }

  return clErr;
}

cl_mem mitk::OclDataSet::GetGPUBuffer()
{
  // clGetMemObjectInfo()
  cl_mem_object_type memInfo;
  cl_int clErr = 0;

  // query image object info only if already initialized
  if( this->m_gpuBuffer )
  {
    clErr = clGetMemObjectInfo(this->m_gpuBuffer, CL_MEM_TYPE, sizeof(cl_mem_object_type), &memInfo, nullptr );
    CHECK_OCL_ERR(clErr);
  }

  MITK_INFO << "Querying info for object, recieving: " << memInfo;

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
  char* data = new char[m_bufferSize * m_BpE];

  // debug info
  oclPrintMemObjectInfo( m_gpuBuffer );

  clErr = clEnqueueReadBuffer( gpuComQueue, m_gpuBuffer, CL_FALSE, 0, m_bufferSize * m_BpE, data ,0, nullptr, nullptr);
  CHECK_OCL_ERR(clErr);

  clFlush( gpuComQueue );
  // the cpu data is same as gpu
  this->m_gpuModified = false;

  return (void*) data;
}

void mitk::OclDataSet::SetSize(unsigned short size)
{
  m_size = size;
}