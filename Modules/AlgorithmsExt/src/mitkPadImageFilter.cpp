/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkPadImageFilter.h"
#include "mitkImageCast.h"
#include <mitkImageAccessByItk.h>
#include "itkBinaryThresholdImageFilter.h"
#include "itkConstantPadImageFilter.h"

mitk::PadImageFilter::PadImageFilter()
{
  this->SetNumberOfIndexedInputs(2);
  this->SetNumberOfRequiredInputs(2);
  m_BinaryFilter = false;

  m_PadConstant = -32766;
  m_LowerThreshold = -32766;
  m_UpperThreshold = -32765;
}

mitk::PadImageFilter::~PadImageFilter()
{
}


template <typename SourceImageType>
void mitk::PadImageFilter::GenerateDataInternal(SourceImageType* sourceItkImage, mitk::Image::Pointer outputImage)
{
  mitk::Image::ConstPointer sourceImage = this->GetInput(0);
  mitk::Image::ConstPointer referenceImage = this->GetInput(1);

  mitk::BaseGeometry* imageGeometry = sourceImage->GetGeometry();
  mitk::Point3D origin = imageGeometry->GetOrigin();
  mitk::Vector3D spacing = imageGeometry->GetSpacing();

  mitk::BaseGeometry* referenceImageGeometry = referenceImage->GetGeometry();
  mitk::Point3D referenceOrigin = referenceImageGeometry->GetOrigin();

  double outputOrigin[3];
  itk::SizeValueType padLowerBound[3];
  itk::SizeValueType padUpperBound[3];

  int i;
  for (i = 0; i < 3; ++i)
  {
    outputOrigin[i] = referenceOrigin[i];

    padLowerBound[i] = static_cast<unsigned long>((origin[i] - referenceOrigin[i]) / spacing[i] + 0.5);

    padUpperBound[i] = referenceImage->GetDimension(i) - sourceImage->GetDimension(i) - padLowerBound[i];
  }

  // The origin of the input image is passed through the filter and used as
  // output origin as well. Hence, it needs to be overwritten accordingly.
  sourceItkImage->SetOrigin(outputOrigin);

  typedef itk::ConstantPadImageFilter<SourceImageType, SourceImageType> PadFilterType;
  PadFilterType::Pointer padFilter = PadFilterType::New();
  padFilter->SetInput(sourceItkImage);
  padFilter->SetConstant(m_PadConstant);
  padFilter->SetPadLowerBound(padLowerBound);
  padFilter->SetPadUpperBound(padUpperBound);

  // If the Binary flag is set, use an additional binary threshold filter after
  // padding.
  if (m_BinaryFilter)
  {
    typedef itk::Image<unsigned char, 3> BinaryImageType;
    typedef itk::BinaryThresholdImageFilter<SourceImageType, BinaryImageType> BinaryFilterType;
    BinaryFilterType::Pointer binaryFilter = BinaryFilterType::New();

    binaryFilter->SetInput(padFilter->GetOutput());
    binaryFilter->SetLowerThreshold(m_LowerThreshold);
    binaryFilter->SetUpperThreshold(m_UpperThreshold);
    binaryFilter->SetInsideValue(1);
    binaryFilter->SetOutsideValue(0);
    binaryFilter->Update();

    mitk::CastToMitkImage(binaryFilter->GetOutput(), outputImage);
  }
  else
  {
    padFilter->Update();
    mitk::CastToMitkImage(padFilter->GetOutput(), outputImage);
  }
}

void mitk::PadImageFilter::GenerateData()
{
  mitk::Image::Pointer image = this->GetInput(0);
  mitk::Image::Pointer outputImage = this->GetOutput();

  AccessFixedDimensionByItk_1(image, GenerateDataInternal, 3, outputImage);

  outputImage->SetRequestedRegionToLargestPossibleRegion();
}
