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
#include "./OpenCLFilter/mitkPhotoacousticOCLDelayCalculation.h"
#include "usServiceReference.h"
#include "mitkImageReadAccessor.h"

mitk::OCLDelayCalculation::OCLDelayCalculation()
  : m_PixelCalculation(NULL)
{
  this->AddSourceFile("DelayCalculation.cl");
  this->m_FilterID = "DelayCalculation";

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

  unsigned int gridDim[3] = { m_Conf.inputDim[0], m_Conf.SamplesPerLine, 1 };
  m_BufferSize = gridDim[0] * gridDim[1] * 1;

  try
  {
    this->InitExecNoInput(this->m_PixelCalculation, gridDim, m_BufferSize, sizeof(unsigned short));
  }
  catch (const mitk::Exception& e)
  {
    MITK_ERROR << "Caught exception while initializing filter: " << e.what();
    return;
  }

  if (m_Conf.DelayCalculationMethod == BeamformingSettings::DelayCalc::QuadApprox)
    m_DelayMultiplicatorRaw = pow(1 / (m_Conf.TimeSpacing*m_Conf.SpeedOfSound) * m_Conf.Pitch * m_Conf.TransducerElements / m_Conf.inputDim[0], 2) / 2;
  else if (m_Conf.DelayCalculationMethod == BeamformingSettings::DelayCalc::Spherical)
    m_DelayMultiplicatorRaw = 1 / (m_Conf.TimeSpacing*m_Conf.SpeedOfSound) * (m_Conf.Pitch*m_Conf.TransducerElements);

  m_IsPAImage = m_Conf.isPhotoacousticImage;
  
  clErr = clSetKernelArg(this->m_PixelCalculation, 1, sizeof(cl_mem), &(this->m_UsedLines));
  clErr |= clSetKernelArg(this->m_PixelCalculation, 2, sizeof(cl_uint), &(this->m_Conf.inputDim[0]));
  clErr |= clSetKernelArg(this->m_PixelCalculation, 3, sizeof(cl_uint), &(this->m_Conf.inputDim[1]));
  clErr |= clSetKernelArg(this->m_PixelCalculation, 4, sizeof(cl_uint), &(this->m_Conf.ReconstructionLines));
  clErr |= clSetKernelArg(this->m_PixelCalculation, 5, sizeof(cl_uint), &(this->m_Conf.SamplesPerLine));
  clErr |= clSetKernelArg(this->m_PixelCalculation, 6, sizeof(cl_char), &(this->m_IsPAImage));
  clErr |= clSetKernelArg(this->m_PixelCalculation, 7, sizeof(cl_float), &(this->m_DelayMultiplicatorRaw));
  
  CHECK_OCL_ERR(clErr);

  // execute the filter on a 3D NDRange
  this->ExecuteKernel(m_PixelCalculation, 2);

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
    if(m_Conf.DelayCalculationMethod == BeamformingSettings::DelayCalc::QuadApprox)
      this->m_PixelCalculation = clCreateKernel(this->m_ClProgram, "ckDelayCalculationQuad", &clErr);
    if (m_Conf.DelayCalculationMethod == BeamformingSettings::DelayCalc::Spherical)
      this->m_PixelCalculation = clCreateKernel(this->m_ClProgram, "ckDelayCalculationSphe", &clErr);
    buildErr |= CHECK_OCL_ERR(clErr);
  }
  return (OclFilter::IsInitialized() && buildErr);
}