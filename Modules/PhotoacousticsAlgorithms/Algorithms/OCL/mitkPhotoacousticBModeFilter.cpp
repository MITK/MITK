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

#include "mitkPhotoacousticBModeFilter.h"
#include "usServiceReference.h"

mitk::PhotoacousticBModeFilter::PhotoacousticBModeFilter()
  : m_PixelCalculation(NULL)
{
  this->AddSourceFile("BModeAbs.cl");
  this->AddSourceFile("BModeAbsLog.cl");

  this->m_FilterID = "PixelCalculation";
}

mitk::PhotoacousticBModeFilter::~PhotoacousticBModeFilter()
{
  if (this->m_PixelCalculation)
  {
    clReleaseKernel(m_PixelCalculation);
  }
}

void mitk::PhotoacousticBModeFilter::Update()
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

void mitk::PhotoacousticBModeFilter::Execute()
{
  try
  {
    this->InitExec(this->m_PixelCalculation);
  }
  catch (const mitk::Exception& e)
  {
    MITK_ERROR << "Catched exception while initializing filter: " << e.what();
    return;
  }

  // execute the filter on a 3D NDRange
  this->ExecuteKernel(m_PixelCalculation, 3);

  // signalize the GPU-side data changed
  m_Output->Modified(GPU_DATA);
}

us::Module *mitk::PhotoacousticBModeFilter::GetModule()
{
  return us::GetModuleContext()->GetModule();
}

bool mitk::PhotoacousticBModeFilter::Initialize()
{
  bool buildErr = true;
  cl_int clErr = 0;

  if (OclFilter::Initialize())
  {
    if(m_UseLogFilter)
      this->m_PixelCalculation = clCreateKernel(this->m_ClProgram, "ckBmodeAbsLog", &clErr);
    else
      this->m_PixelCalculation = clCreateKernel(this->m_ClProgram, "ckBmodeAbs", &clErr);
    buildErr |= CHECK_OCL_ERR(clErr);
  }
  return (OclFilter::IsInitialized() && buildErr);
}

void mitk::PhotoacousticBModeFilter::SetInput(mitk::Image::Pointer image)
{
  if (image->GetDimension() != 3)
  {
    mitkThrowException(mitk::Exception) << "Input for " << this->GetNameOfClass() <<
      " is not 3D. The filter only supports 3D. Please change your input.";
  }
  OclImageToImageFilter::SetInput(image);
}
