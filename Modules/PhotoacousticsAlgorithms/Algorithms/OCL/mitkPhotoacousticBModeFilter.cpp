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
#include <mitkImageReadAccessor.h>

#ifdef PHOTOACOUSTICS_USE_GPU

mitk::PhotoacousticOCLBModeFilter::PhotoacousticOCLBModeFilter()
  : m_PixelCalculation(NULL)
{
  this->AddSourceFile("BModeAbs.cl");
  this->AddSourceFile("BModeAbsLog.cl");

  this->m_FilterID = "PixelCalculation";
}

mitk::PhotoacousticOCLBModeFilter::~PhotoacousticOCLBModeFilter()
{
  if (this->m_PixelCalculation)
  {
    clReleaseKernel(m_PixelCalculation);
  }
}

void mitk::PhotoacousticOCLBModeFilter::Update()
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

void mitk::PhotoacousticOCLBModeFilter::Execute()
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

us::Module *mitk::PhotoacousticOCLBModeFilter::GetModule()
{
  return us::GetModuleContext()->GetModule();
}

bool mitk::PhotoacousticOCLBModeFilter::Initialize()
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

void mitk::PhotoacousticOCLBModeFilter::SetInput(mitk::Image::Pointer image)
{
  if (image->GetDimension() != 3)
  {
    mitkThrowException(mitk::Exception) << "Input for " << this->GetNameOfClass() <<
      " is not 3D. The filter only supports 3D. Please change your input.";
  }
  OclImageToImageFilter::SetInput(image);
}

#endif


mitk::PhotoacousticBModeFilter::PhotoacousticBModeFilter() : m_UseLogFilter(false)
{
  this->SetNumberOfIndexedInputs(1);
  this->SetNumberOfRequiredInputs(1);
}

mitk::PhotoacousticBModeFilter::~PhotoacousticBModeFilter()
{
}

void mitk::PhotoacousticBModeFilter::GenerateInputRequestedRegion()
{
  Superclass::GenerateInputRequestedRegion();

  mitk::Image* output = this->GetOutput();
  mitk::Image* input = const_cast<mitk::Image *> (this->GetInput());
  if (!output->IsInitialized())
  {
    return;
  }

  input->SetRequestedRegionToLargestPossibleRegion();

  //GenerateTimeInInputRegion(output, input);
}

void mitk::PhotoacousticBModeFilter::GenerateOutputInformation()
{
  mitk::Image::ConstPointer input = this->GetInput();
  mitk::Image::Pointer output = this->GetOutput();

  if ((output->IsInitialized()) && (this->GetMTime() <= m_TimeOfHeaderInitialization.GetMTime()))
    return;

  itkDebugMacro(<< "GenerateOutputInformation()");

  output->Initialize(input);
  output->GetGeometry()->SetSpacing(input->GetGeometry()->GetSpacing());
  output->GetGeometry()->Modified();
  output->SetPropertyList(input->GetPropertyList()->Clone());

  m_TimeOfHeaderInitialization.Modified();
}

void mitk::PhotoacousticBModeFilter::GenerateData()
{
  GenerateOutputInformation();
  mitk::Image::Pointer input = this->GetInput();
  mitk::Image::Pointer output = this->GetOutput();

  if (!output->IsInitialized())
    return;

  mitk::ImageReadAccessor reader(input);

  unsigned int size = output->GetDimension(0) * output->GetDimension(1) * output->GetDimension(2);

  float* InputData = (float*)const_cast<void*>(reader.GetData());
  float* OutputData = new float[size];
  if(!m_UseLogFilter)
    for (unsigned int i = 0; i < size; ++i)
    {
      OutputData[i] = abs(InputData[i]);
    }
  else
  {
    for (unsigned int i = 0; i < size; ++i)
    {
      OutputData[i] = log(abs(InputData[i]));
    }
  }

  output->SetImportVolume(OutputData, 0, 0, mitk::Image::ImportMemoryManagementType::ManageMemory);

  m_TimeOfHeaderInitialization.Modified();
}