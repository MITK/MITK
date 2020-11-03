/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkOclDataSetToDataSetFilter.h"
#include "mitkOclDataSet.h"

#include "mitkException.h"

mitk::OclDataSetToDataSetFilter::OclDataSetToDataSetFilter()
{
  m_Output = mitk::OclDataSet::New();
}

mitk::OclDataSetToDataSetFilter::~OclDataSetToDataSetFilter()
{
}

mitk::OclDataSet::Pointer mitk::OclDataSetToDataSetFilter::GetGPUOutput()
{
  return this->m_Output;
}

void* mitk::OclDataSetToDataSetFilter::GetOutput()
{
  void* pData = m_Output->TransferDataToCPU(m_CommandQue);
  return pData;
}

int mitk::OclDataSetToDataSetFilter::GetBytesPerElem()
{
  return this->m_CurrentSizeOutput;
}

bool mitk::OclDataSetToDataSetFilter::InitExec(cl_kernel ckKernel, unsigned int* dimensions, size_t outputDataSize, unsigned int outputBpE)
{
  cl_int clErr = 0;

  if (m_Input.IsNull())
    mitkThrow() << "Input DataSet is null.";

  // get DataSet size once
  const unsigned int uiDataSetWidth = dimensions[0];
  const unsigned int uiDataSetHeight = dimensions[1];
  const unsigned int uiDataSetDepth = dimensions[2];

  // compute work sizes
  this->SetWorkingSize(8, uiDataSetWidth, 8, uiDataSetHeight, 8, uiDataSetDepth);

  cl_mem clBuffIn = m_Input->GetGPUBuffer();
  cl_mem clBuffOut = m_Output->GetGPUBuffer();

  if (!clBuffIn)
  {
    if (m_Input->TransferDataToGPU(m_CommandQue) != CL_SUCCESS)
    {
      mitkThrow() << "Could not create / initialize gpu DataSet.";
    }

    clBuffIn = m_Input->GetGPUBuffer();
  }

  // output DataSet not initialized or output buffer size changed
  if (!clBuffOut || (size_t)m_Output->GetBufferSize() != outputDataSize)
  {
    MITK_DEBUG << "Create GPU DataSet call " << uiDataSetWidth << "x" << uiDataSetHeight << "x" << uiDataSetDepth;
    MITK_INFO << "Create GPU Buffer of size " << outputDataSize * outputBpE / 1024.f / 1024.f << "MB";
    m_Output->SetBpE(outputBpE);
    m_Output->SetBufferSize(outputDataSize);
    clBuffOut = m_Output->CreateGPUBuffer();
    m_CurrentSizeOutput = outputBpE;
  }

  clErr = 0;
  clErr = clSetKernelArg(ckKernel, 0, sizeof(cl_mem), &clBuffIn);
  clErr |= clSetKernelArg(ckKernel, 1, sizeof(cl_mem), &clBuffOut);
  CHECK_OCL_ERR(clErr);

  if (clErr != CL_SUCCESS)
    mitkThrow() << "OpenCL Part initialization failed with " << GetOclErrorAsString(clErr);

  return(clErr == CL_SUCCESS);
}

bool mitk::OclDataSetToDataSetFilter::InitExecNoInput(cl_kernel ckKernel, unsigned int* dimensions, size_t outputDataSize, unsigned int outputBpE)
{
  cl_int clErr = 0;

  // get DataSet size once
  const unsigned int uiDataSetWidth = dimensions[0];
  const unsigned int uiDataSetHeight = dimensions[1];
  const unsigned int uiDataSetDepth = dimensions[2];

  // compute work sizes
  this->SetWorkingSize(8, uiDataSetWidth, 8, uiDataSetHeight, 8, uiDataSetDepth);

  cl_mem clBuffOut = m_Output->GetGPUBuffer();

  // output DataSet not initialized or output buffer size changed
  if (!clBuffOut || (size_t)m_Output->GetBufferSize() != outputDataSize)
  {
    MITK_DEBUG << "Create GPU DataSet call " << uiDataSetWidth << "x" << uiDataSetHeight << "x" << uiDataSetDepth;
    m_Output->SetBpE(outputBpE);
    m_Output->SetBufferSize(outputDataSize);
    clBuffOut = m_Output->CreateGPUBuffer();
    m_CurrentSizeOutput = outputBpE;
  }

  clErr = clSetKernelArg(ckKernel, 0, sizeof(cl_mem), &clBuffOut);
  CHECK_OCL_ERR(clErr);

  if (clErr != CL_SUCCESS)
    mitkThrow() << "OpenCL Part initialization failed with " << GetOclErrorAsString(clErr);

  return(clErr == CL_SUCCESS);
}
