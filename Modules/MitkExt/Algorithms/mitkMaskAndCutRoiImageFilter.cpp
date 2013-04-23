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
#include "mitkMaskAndCutRoiImageFilter.h"

#include "mitkBoundingObjectToSegmentationFilter.h"
#include "mitkImageCast.h"

mitk::MaskAndCutRoiImageFilter::MaskAndCutRoiImageFilter()
{
  this->SetNumberOfRequiredInputs(2);
  m_CropFilter = mitk::AutoCropImageFilter::New();
  m_RoiFilter = ROIFilterType::New();
  m_MaskFilter = mitk::MaskImageFilter::New();
}

mitk::MaskAndCutRoiImageFilter::~MaskAndCutRoiImageFilter()
{

}

void mitk::MaskAndCutRoiImageFilter::SetRegionOfInterest(mitk::BaseData* roi)
{
  mitk::Image::Pointer image = dynamic_cast<mitk::Image*> (roi);

  if (image.IsNotNull())
  {
    this->SetInput(1, image);
    return;
  }

  mitk::BoundingObject::Pointer boundingObject = dynamic_cast<mitk::BoundingObject*> (roi);
  if (boundingObject.IsNotNull() && this->GetInput(0) != NULL)
  {
    mitk::BoundingObjectToSegmentationFilter::Pointer filter = mitk::BoundingObjectToSegmentationFilter::New();
    filter->SetBoundingObject( boundingObject);
    filter->SetInput(this->GetInput(0));
    filter->Update();

    this->SetInput(1, filter->GetOutput());
    return;
  }
}

mitk::Image::Pointer mitk::MaskAndCutRoiImageFilter::GetOutput()
{
    return m_outputImage;
}

void mitk::MaskAndCutRoiImageFilter::GenerateData()
{

  mitk::Image::ConstPointer  inputImage = this->GetInput(0);
  mitk::Image::ConstPointer  maskImage = this->GetInput(1);
  //mitk::Image::Pointer outputImage = this->GetOutput();
  //temporary fix for bug #
  m_outputImage = this->GetOutput();

  ItkImageType::Pointer itkImage = ItkImageType::New();
  mitk::Image::Pointer tmpImage = mitk::Image::New();

  m_CropFilter->SetInput(maskImage);
  m_CropFilter->SetBackgroundValue(0);
  m_CropFilter->Update();

  RegionType region = m_CropFilter->GetCroppingRegion();

  mitk::CastToItkImage(inputImage, itkImage);
  m_RoiFilter->SetInput(itkImage);
  m_RoiFilter->SetRegionOfInterest(region);
  m_RoiFilter->Update();

  mitk::CastToMitkImage(m_RoiFilter->GetOutput(), tmpImage);


  m_MaskFilter->SetInput(0,tmpImage);
  m_MaskFilter->SetMask(m_CropFilter->GetOutput());

  m_MaskFilter->SetOutsideValue(-32765);
  m_MaskFilter->Update();
  m_MaxValue = m_MaskFilter->GetMaxValue();
  m_MinValue = m_MaskFilter->GetMinValue();

  //temporary fix for bug #
  m_outputImage = m_MaskFilter->GetOutput();
  m_outputImage->DisconnectPipeline();
}
