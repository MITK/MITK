/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkMaskImageFilter.h"
#include "mitkImageTimeSelector.h"
#include "mitkProperties.h"
#include "mitkTimeHelper.h"
#include "mitkImageTimeSelector.h"

#include "mitkImageAccessByItk.h"
#include "mitkImageToItk.h"

#include "itkMaskImageFilter.h"

#include <limits>

mitk::MaskImageFilter::MaskImageFilter()
{
  this->SetNumberOfIndexedInputs(2);
  this->SetNumberOfRequiredInputs(2);
  m_OverrideOutsideValue = false;
  m_OutsideValue = 0;
  m_CurrentOutputTS = 0;
}

mitk::MaskImageFilter::~MaskImageFilter()
{
}

void mitk::MaskImageFilter::SetMask(const mitk::Image *mask)
{
  // Process object is not const-correct so the const_cast is required here
  auto nonconstMask = const_cast<mitk::Image *>(mask);
  this->ProcessObject::SetNthInput(1, nonconstMask);
}

const mitk::Image *mitk::MaskImageFilter::GetMask() const
{
  return this->GetInput(1);
}

mitk::Image* mitk::MaskImageFilter::GetMask()
{
  return this->GetInput(1);
}

void mitk::MaskImageFilter::GenerateInputRequestedRegion()
{
  Superclass::GenerateInputRequestedRegion();

  mitk::Image *output = this->GetOutput();
  mitk::Image *input = this->GetInput();
  mitk::Image *mask = this->GetMask();
  if ((output->IsInitialized() == false) || (mask == nullptr) || (mask->GetTimeGeometry()->CountTimeSteps() == 0))
    return;

  input->SetRequestedRegionToLargestPossibleRegion();
  mask->SetRequestedRegionToLargestPossibleRegion();

  GenerateTimeInInputRegion(output, input);
}

void mitk::MaskImageFilter::GenerateOutputInformation()
{
  mitk::Image::ConstPointer input = this->GetInput();
  mitk::Image::Pointer output = this->GetOutput();

  if ((output->IsInitialized()) && (this->GetMTime() <= m_TimeOfHeaderInitialization.GetMTime()))
    return;

  itkDebugMacro(<< "GenerateOutputInformation()");

  output->Initialize(input->GetPixelType(), *input->GetTimeGeometry());

  output->SetPropertyList(input->GetPropertyList()->Clone());

  m_TimeOfHeaderInitialization.Modified();
}

template <typename TPixel1, unsigned int VImageDimension1, typename TPixel2, unsigned int VImageDimension2>
void mitk::MaskImageFilter::InternalComputeMask(itk::Image<TPixel1, VImageDimension1>* itkInput, itk::Image<TPixel2,VImageDimension2>* itkMask)
{
  using MaskFilterType = itk::MaskImageFilter< itk::Image<TPixel1, VImageDimension1>, itk::Image<TPixel2, VImageDimension2>>;

  auto maskFilter = MaskFilterType::New();
  maskFilter->SetInput(itkInput);
  maskFilter->SetMaskImage(itkMask);
  maskFilter->SetOutsideValue(m_OutsideValue);
  maskFilter->Update();

  auto output = maskFilter->GetOutput();

  // re-import to MITK
  auto mitkOutput = this->GetOutput();
  mitkOutput->SetVolume(output->GetBufferPointer(), m_CurrentOutputTS);
}

void mitk::MaskImageFilter::GenerateData()
{
  auto input = this->GetInput();
  auto mask = this->GetMask();
  mitk::Image::Pointer output = this->GetOutput();

  if ((output->IsInitialized() == false) || (nullptr == mask) || (mask->GetTimeGeometry()->CountTimeSteps() == 0))
    return;

  mitk::Image::RegionType outputRegion = output->GetRequestedRegion();
  const mitk::TimeGeometry *outputTimeGeometry = output->GetTimeGeometry();
  const mitk::TimeGeometry *inputTimeGeometry = input->GetTimeGeometry();
  const mitk::TimeGeometry *maskTimeGeometry = mask->GetTimeGeometry();

  for (TimeStepType t = 0; t < inputTimeGeometry->CountTimeSteps(); ++t)
  {
    auto timeInMS = outputTimeGeometry->TimeStepToTimePoint(t);

    m_CurrentOutputTS = t;
    auto inputFrame = SelectImageByTimePoint(input, timeInMS);
    auto maskFrame = SelectImageByTimePoint(mask, timeInMS);
    AccessTwoImagesFixedDimensionByItk(inputFrame, maskFrame, InternalComputeMask, 3);
  }

  m_TimeOfHeaderInitialization.Modified();
}
