/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: 27918 $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/
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

void mitk::MaskAndCutRoiImageFilter::SetRegionOfInterest(mitk::Image::Pointer image)
{
  if (image.IsNotNull())
  {
    this->SetInput(1, image);
  }
}

void mitk::MaskAndCutRoiImageFilter::SetRegionOfInterest(mitk::BoundingObject::Pointer boundingObject)
{
  if (boundingObject.IsNotNull() && this->GetInput(0) != NULL)
  {
    mitk::BoundingObjectToSegmentationFilter::Pointer filter = mitk::BoundingObjectToSegmentationFilter::New();
    filter->SetBoundingObject( boundingObject);
    filter->SetInput(this->GetInput(0));
    filter->Update();
    this->SetRegionOfInterest(filter->GetOutput());
  }

}

void mitk::MaskAndCutRoiImageFilter::SetRegionOfInterestByNode(mitk::DataNode::Pointer node)
{
  mitk::BoundingObject::Pointer boundingObject = dynamic_cast<mitk::BoundingObject*> (node->GetData());
  if (boundingObject)
  {
   this->SetRegionOfInterest(boundingObject);
   return;
 }
  mitk::Image::Pointer image = dynamic_cast<mitk::Image*> (node->GetData());
  if (image)
    this->SetRegionOfInterest(image);
}

mitk::Image::Pointer mitk::MaskAndCutRoiImageFilter::GetImage()
{
    return m_outputImage;
}

void mitk::MaskAndCutRoiImageFilter::GenerateData()
{

  mitk::Image::ConstPointer  inputImage = this->GetInput(0);
  mitk::Image::ConstPointer  maskImage = this->GetInput(1);
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

  //maskFilter->SetOutsideValue(0);
  m_MaskFilter->Update();
  m_MaxValue = m_MaskFilter->GetMaxValue();
  m_MinValue = m_MaskFilter->GetMinValue();
  m_outputImage = m_MaskFilter->GetOutput();
  m_outputImage->DisconnectPipeline();
}
