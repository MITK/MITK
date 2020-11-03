/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkBinaryImageToLabelSetImageFilter.h"

#include <mitkImage.h>
#include <mitkLabelSetImage.h>
#include <mitkImageAccessByItk.h>
#include <mitkITKImageImport.h>

//itk
#include <itkBinaryImageToLabelMapFilter.h>
#include <itkLabelMapToLabelImageFilter.h>


  template <typename TPixel, unsigned int VImageDimension>
  void mitk::BinaryImageToLabelSetImageFilter::ApplyBinaryImageToLabelMapFilter(const itk::Image<TPixel, VImageDimension>* inputImage)
  {
    using ImageType = itk::Image<TPixel, VImageDimension>;
    using  BinaryImageToLabelMapFilterType = itk::BinaryImageToLabelMapFilter<ImageType>;
    typename BinaryImageToLabelMapFilterType::Pointer binaryImageToLabelMapFilter = BinaryImageToLabelMapFilterType::New();
    binaryImageToLabelMapFilter->SetInput(inputImage);
    binaryImageToLabelMapFilter->SetInputForegroundValue(m_ForegroundValue);
    binaryImageToLabelMapFilter->SetFullyConnected(m_FullyConnected);

    using LabelMap2ImageType = itk::LabelMapToLabelImageFilter< typename BinaryImageToLabelMapFilterType::OutputImageType, ImageType>;

    typename LabelMap2ImageType::Pointer label2image = LabelMap2ImageType::New();
    label2image->SetInput(binaryImageToLabelMapFilter->GetOutput());
    label2image->Update();
    auto labeledImage = mitk::ImportItkImage(label2image->GetOutput());

    if (m_OutputIsLabelSetImage) {
      auto labeledBinaryImage = mitk::LabelSetImage::New();
      labeledBinaryImage->InitializeByLabeledImage(labeledImage);
      this->SetOutput(MakeNameFromOutputIndex(0), labeledBinaryImage.GetPointer());
    }
    else {
      this->SetOutput(MakeNameFromOutputIndex(0), labeledImage.GetPointer());
    }
  }

  void mitk::BinaryImageToLabelSetImageFilter::VerifyInputImage(const mitk::Image* inputImage)
  {
    if (!inputImage->IsInitialized())
      mitkThrow() << "Input image is not initialized.";

    if (!inputImage->IsVolumeSet())
      mitkThrow() << "Input image volume is not set.";

    auto geometry = inputImage->GetGeometry();

    if (nullptr == geometry || !geometry->IsValid())
      mitkThrow() << "Input image has invalid geometry.";

    if (!geometry->GetImageGeometry())
      mitkThrow() << "Geometry of input image is not an image geometry.";
  }

void mitk::BinaryImageToLabelSetImageFilter::GenerateData()
{
  const auto* inputImage = this->GetInput();
  AccessByItk(inputImage, ApplyBinaryImageToLabelMapFilter);
}

void mitk::BinaryImageToLabelSetImageFilter::SetInput(const InputImageType* image)
{
  if (this->GetInput() == image)
    return;

  Superclass::SetInput(image);
}

void mitk::BinaryImageToLabelSetImageFilter::SetInput(unsigned int index, const InputImageType* image)
{
  if (0 != index)
    mitkThrow() << "Input index " << index << " is invalid.";

  this->SetInput(image);
}

void mitk::BinaryImageToLabelSetImageFilter::VerifyInputInformation()
{
  Superclass::VerifyInputInformation();

  VerifyInputImage(this->GetInput());
}
