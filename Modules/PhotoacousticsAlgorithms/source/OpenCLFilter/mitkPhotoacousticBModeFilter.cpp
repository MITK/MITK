/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "./OpenCLFilter/mitkPhotoacousticBModeFilter.h"
#include "usServiceReference.h"
#include <mitkImageReadAccessor.h>

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

  mitk::Image::Pointer output = this->GetOutput();
  mitk::Image::Pointer input = this->GetInput();

  if (!output->IsInitialized())
    return;

  input->SetRequestedRegionToLargestPossibleRegion();
}

void mitk::PhotoacousticBModeFilter::GenerateOutputInformation()
{
  mitk::Image::ConstPointer input = this->GetInput();
  mitk::Image::Pointer output = this->GetOutput();

  if ((output->IsInitialized()) && (this->GetMTime() <= m_TimeOfHeaderInitialization.GetMTime()))
    return;

  output->Initialize(input->GetPixelType(), input->GetDimension(), input->GetDimensions());
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

  const float* InputData = (const float*)(reader.GetData());
  float* OutputData = new float[size];
  if (!m_UseLogFilter)
    for (unsigned int i = 0; i < size; ++i)
    {
      OutputData[i] = std::abs(InputData[i]);
    }
  else
  {
    for (unsigned int i = 0; i < size; ++i)
    {
      OutputData[i] = log(std::abs(InputData[i]));
    }
  }

  output->SetImportVolume(OutputData, 0, 0, mitk::Image::ImportMemoryManagementType::CopyMemory);
  delete[] OutputData;
  m_TimeOfHeaderInitialization.Modified();
}
