/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#define _USE_MATH_DEFINES

#include <cmath>
#include "./OpenCLFilter/mitkPhotoacousticOCLDelayCalculation.h"
#include "usServiceReference.h"
#include "mitkImageReadAccessor.h"

mitk::OCLDelayCalculation::OCLDelayCalculation(mitk::BeamformingSettings::Pointer settings)
  : m_PixelCalculation(NULL),
  m_Conf(settings)
{
  this->AddSourceFile("DelayCalculation.cl");
  this->m_FilterID = "DelayCalculation";

  m_ChunkSize[0] = 128;
  m_ChunkSize[1] = 128;
  m_ChunkSize[2] = 8;

  this->Initialize();
}

mitk::OCLDelayCalculation::~OCLDelayCalculation()
{
  if (this->m_PixelCalculation)
  {
    clReleaseKernel(m_PixelCalculation);
  }
}

void mitk::OCLDelayCalculation::Update()
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

void mitk::OCLDelayCalculation::Execute()
{
  cl_int clErr = 0;

  unsigned int gridDim[3] = { m_Conf->GetReconstructionLines() / 2, m_Conf->GetSamplesPerLine(), 1 };
  m_BufferSize = gridDim[0] * gridDim[1] * 1;

  try
  {
    this->InitExecNoInput(this->m_PixelCalculation, gridDim, m_BufferSize, sizeof(unsigned short));
  }
  catch (const mitk::Exception& e)
  {
    MITK_ERROR << "Caught exception while initializing Delay Calculation filter: " << e.what();
    return;
  }

  // This calculation is the same for all kernels, so for performance reasons simply perform it here instead of within the kernels
  m_DelayMultiplicatorRaw = 1 / (m_Conf->GetTimeSpacing()*m_Conf->GetSpeedOfSound()) *
    (m_Conf->GetPitchInMeters()*(float)m_Conf->GetTransducerElements());

  // as openCL does not support bool as a kernel argument, we need to buffer this value in a char...
  m_IsPAImage = m_Conf->GetIsPhotoacousticImage();

  unsigned int reconstructionLines = this->m_Conf->GetReconstructionLines();
  unsigned int samplesperLine = this->m_Conf->GetSamplesPerLine();

  float totalSamples_i = (float)(m_Conf->GetReconstructionDepth()) / (float)(m_Conf->GetSpeedOfSound() * m_Conf->GetTimeSpacing());
  totalSamples_i = totalSamples_i <= m_Conf->GetInputDim()[1] ? totalSamples_i : m_Conf->GetInputDim()[1];

  clErr = clSetKernelArg(this->m_PixelCalculation, 1, sizeof(cl_mem), &(this->m_UsedLines));
  clErr |= clSetKernelArg(this->m_PixelCalculation, 2, sizeof(cl_uint), &(this->m_Conf->GetInputDim()[0]));
  clErr |= clSetKernelArg(this->m_PixelCalculation, 3, sizeof(cl_uint), &(this->m_Conf->GetInputDim()[1]));
  clErr |= clSetKernelArg(this->m_PixelCalculation, 4, sizeof(cl_uint), &(reconstructionLines));
  clErr |= clSetKernelArg(this->m_PixelCalculation, 5, sizeof(cl_uint), &(samplesperLine));
  clErr |= clSetKernelArg(this->m_PixelCalculation, 6, sizeof(cl_char), &(this->m_IsPAImage));
  clErr |= clSetKernelArg(this->m_PixelCalculation, 7, sizeof(cl_float), &(this->m_DelayMultiplicatorRaw));
  clErr |= clSetKernelArg(this->m_PixelCalculation, 8, sizeof(cl_float), &(totalSamples_i));

  CHECK_OCL_ERR(clErr);

  // execute the filter on a 3D NDRange
  if (!this->ExecuteKernelChunksInBatches(m_PixelCalculation, 2, m_ChunkSize, 16, 50))
    mitkThrow() << "openCL Error when executing Kernel";
  // signalize the GPU-side data changed
  m_Output->Modified(GPU_DATA);
}

us::Module *mitk::OCLDelayCalculation::GetModule()
{
  return us::GetModuleContext()->GetModule();
}

bool mitk::OCLDelayCalculation::Initialize()
{
  bool buildErr = true;
  cl_int clErr = 0;

  if (OclFilter::Initialize())
  {
    this->m_PixelCalculation = clCreateKernel(this->m_ClProgram, "ckDelayCalculationSphe", &clErr);
    buildErr |= CHECK_OCL_ERR(clErr);
  }
  return (OclFilter::IsInitialized() && buildErr);
}
