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

#define _USE_MATH_DEFINES

#include <cmath>
#include "./OpenCLFilter/mitkPhotoacousticOCLMemoryLocSum.h"
#include "usServiceReference.h"
#include "mitkImageReadAccessor.h"

mitk::OCLMemoryLocSum::OCLMemoryLocSum()
  : m_PixelCalculation(NULL), m_Sum(0)
{
  this->AddSourceFile("MemoryLocSum.cl");
  this->m_FilterID = "MemoryLocSum";

  this->Initialize();
}

mitk::OCLMemoryLocSum::~OCLMemoryLocSum()
{
  if (this->m_PixelCalculation)
  {
    clReleaseKernel(m_PixelCalculation);
  }
}

void mitk::OCLMemoryLocSum::Update()
{
  //Check if context & program available
  if (!this->Initialize())
  {
    us::ServiceReference<OclResourceService> ref = GetModuleContext()->GetServiceReference<OclResourceService>();
    OclResourceService* resources = GetModuleContext()->GetService<OclResourceService>(ref);

    // clean-up also the resources
    resources->InvalidateStorage();
    mitkThrow() << "Filter is not initialized. Cannot update.";
  }
  else {
    // Execute
    this->Execute();
  }
}

void mitk::OCLMemoryLocSum::Execute()
{
  cl_int clErr = 0;

  unsigned int gridDim[3] = { m_Dim[0], m_Dim[1], 1 };
  size_t outputSize = gridDim[0] * gridDim[1] * 1;

  try
  {
    this->InitExec(this->m_PixelCalculation, gridDim, outputSize, sizeof(unsigned int));
  }
  catch (const mitk::Exception& e)
  {
    MITK_ERROR << "Caught exception while initializing filter: " << e.what();
    return;
  }

  us::ServiceReference<OclResourceService> ref = GetModuleContext()->GetServiceReference<OclResourceService>();
  OclResourceService* resources = GetModuleContext()->GetService<OclResourceService>(ref);
  cl_context gpuContext = resources->GetContext();

  cl_mem cl_Sum = clCreateBuffer(gpuContext, CL_MEM_WRITE_ONLY, sizeof(unsigned int), NULL, &clErr);
  CHECK_OCL_ERR(clErr);

  clErr = clSetKernelArg(this->m_PixelCalculation, 2, sizeof(cl_mem), &cl_Sum);

  CHECK_OCL_ERR(clErr);

  // execute the filter on a 3D NDRange
  this->ExecuteKernel(m_PixelCalculation, 2);

  // signalize the GPU-side data changed
  m_Output->Modified(GPU_DATA);

  char* data = new char[1 * sizeof(unsigned int)];

  clErr = clEnqueueReadBuffer(m_CommandQue, cl_Sum, CL_FALSE, 0, 1 * sizeof(unsigned int), data, 0, nullptr, nullptr);
  CHECK_OCL_ERR(clErr);

  clFlush(m_CommandQue);

  m_Sum = ((unsigned int*)data)[0];
}

us::Module *mitk::OCLMemoryLocSum::GetModule()
{
  return us::GetModuleContext()->GetModule();
}

bool mitk::OCLMemoryLocSum::Initialize()
{
  bool buildErr = true;
  cl_int clErr = 0;

  if (OclFilter::Initialize())
  {
    this->m_PixelCalculation = clCreateKernel(this->m_ClProgram, "ckMemoryLocSum", &clErr);
    buildErr |= CHECK_OCL_ERR(clErr);
  }
  return (OclFilter::IsInitialized() && buildErr);
}