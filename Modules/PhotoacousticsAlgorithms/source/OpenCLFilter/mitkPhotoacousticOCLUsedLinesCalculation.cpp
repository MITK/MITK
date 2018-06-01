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
#if defined(PHOTOACOUSTICS_USE_GPU) || DOXYGEN

#include "./OpenCLFilter/mitkPhotoacousticOCLUsedLinesCalculation.h"
#include "usServiceReference.h"
#include "mitkImageReadAccessor.h"

mitk::OCLUsedLinesCalculation::OCLUsedLinesCalculation(mitk::BeamformingSettings::Pointer settings)
  : m_PixelCalculation(NULL),
  m_Conf(settings)
{
  this->AddSourceFile("UsedLinesCalculation.cl");
  this->m_FilterID = "UsedLinesCalculation";

  m_ChunkSize[0] = 128;
  m_ChunkSize[1] = 128;
  m_ChunkSize[2] = 8;

  this->Initialize();
}

mitk::OCLUsedLinesCalculation::~OCLUsedLinesCalculation()
{
  if (this->m_PixelCalculation)
  {
    clReleaseKernel(m_PixelCalculation);
  }
}

void mitk::OCLUsedLinesCalculation::Update()
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

void mitk::OCLUsedLinesCalculation::Execute()
{
  cl_int clErr = 0;

  unsigned int gridDim[3] = { m_Conf->GetReconstructionLines(), m_Conf->GetSamplesPerLine(), 1 };
  size_t outputSize = gridDim[0] * gridDim[1] * 3;

  try
  {
    this->InitExecNoInput(this->m_PixelCalculation, gridDim, outputSize, sizeof(unsigned short));
  }
  catch (const mitk::Exception& e)
  {
    MITK_ERROR << "Caught exception while initializing UsedLines filter: " << e.what();
    return;
  }

  // This calculation is the same for all kernels, so for performance reasons simply perform it here instead of within the kernels
  m_part = (tan(m_Conf->GetAngle() / 360 * 2 * itk::Math::pi) *
    ((m_Conf->GetSpeedOfSound() * m_Conf->GetTimeSpacing())) /
    (m_Conf->GetPitchInMeters() * m_Conf->GetTransducerElements())) * m_Conf->GetInputDim()[0];

  unsigned int reconLines = this->m_Conf->GetReconstructionLines();
  unsigned int samplesPerLine = this->m_Conf->GetSamplesPerLine();
  char isPAImage = this->m_Conf->GetIsPhotoacousticImage();

  float percentOfImageReconstructed = (float)(m_Conf->GetReconstructionDepth()) /
    (float)(m_Conf->GetInputDim()[1] * m_Conf->GetSpeedOfSound() * m_Conf->GetTimeSpacing() / (float)(2 - (int)m_Conf->GetIsPhotoacousticImage()));
  percentOfImageReconstructed = percentOfImageReconstructed <= 1 ? percentOfImageReconstructed : 1;

  clErr = clSetKernelArg(this->m_PixelCalculation, 1, sizeof(cl_float), &(this->m_part));
  clErr |= clSetKernelArg(this->m_PixelCalculation, 2, sizeof(cl_uint), &(this->m_Conf->GetInputDim()[0]));
  clErr |= clSetKernelArg(this->m_PixelCalculation, 3, sizeof(cl_uint), &(this->m_Conf->GetInputDim()[1]));
  clErr |= clSetKernelArg(this->m_PixelCalculation, 4, sizeof(cl_uint), &(reconLines));
  clErr |= clSetKernelArg(this->m_PixelCalculation, 5, sizeof(cl_uint), &(samplesPerLine));
  clErr |= clSetKernelArg(this->m_PixelCalculation, 6, sizeof(cl_char), &(isPAImage));
  clErr |= clSetKernelArg(this->m_PixelCalculation, 7, sizeof(cl_float), &(percentOfImageReconstructed));

  CHECK_OCL_ERR(clErr);

  // execute the filter on a 2D NDRange
  if (!this->ExecuteKernelChunksInBatches(m_PixelCalculation, 2, m_ChunkSize, 16, 50))
    mitkThrow() << "openCL Error when executing Kernel";

  // signalize the GPU-side data changed
  m_Output->Modified(GPU_DATA);
}

us::Module *mitk::OCLUsedLinesCalculation::GetModule()
{
  return us::GetModuleContext()->GetModule();
}

bool mitk::OCLUsedLinesCalculation::Initialize()
{
  bool buildErr = true;
  cl_int clErr = 0;

  if (OclFilter::Initialize())
  {
    this->m_PixelCalculation = clCreateKernel(this->m_ClProgram, "ckUsedLines", &clErr);
    buildErr |= CHECK_OCL_ERR(clErr);
  }
  return (OclFilter::IsInitialized() && buildErr);
}
#endif
