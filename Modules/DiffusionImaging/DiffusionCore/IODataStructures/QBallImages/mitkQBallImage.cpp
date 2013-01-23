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

#include "mitkQBallImage.h"
#include "mitkImageCast.h"
#include "itkImage.h"
#include "mitkImageVtkAccessor.h"
#include "itkQBallToRgbImageFilter.h"

mitk::QBallImage::QBallImage() : Image()
{
  m_RgbImage = 0;
}

mitk::QBallImage::~QBallImage()
{

}

mitk::ImageVtkAccessor* mitk::QBallImage::GetVtkImageData(int t, int n)
{
  if(m_RgbImage.IsNull())
    ConstructRgbImage();
  return m_RgbImage->GetVtkImageData(t,n);
}

void mitk::QBallImage::ConstructRgbImage()
{
  typedef itk::Image<itk::Vector<float,QBALL_ODFSIZE>,3> ImageType;
  typedef itk::QBallToRgbImageFilter<ImageType> FilterType;
  FilterType::Pointer filter = FilterType::New();

  ImageType::Pointer itkvol = ImageType::New();
  mitk::CastToItkImage<ImageType>(this, itkvol);
  filter->SetInput(itkvol);
  filter->Update();

  m_RgbImage = mitk::Image::New();
  m_RgbImage->InitializeByItk( filter->GetOutput() );
  m_RgbImage->SetVolume( filter->GetOutput()->GetBufferPointer() );
}

mitk::ImageVtkAccessor* mitk::QBallImage::GetNonRgbVtkImageData(int t, int n)
{
  return Superclass::GetVtkImageData(t,n);
}

//
//void mitk::QBallImage::CopyConstruct(mitk::Image::Pointer img)
//{
//  m_LargestPossibleRegion = img->GetLargestPossibleRegion();
//  m_RequestedRegion = img->GetRequestedRegion();
//  m_Channels.push_back(img->GetChannelData(0).GetPointer());
//  m_Volumes.push_back(img->GetVolumeData(0).GetPointer());
//  m_Slices.push_back(img->GetSliceData(0).GetPointer());
//  m_Dimension = img->GetDimension();
//  m_Dimensions = img->GetDimensions();
//  m_PixelType = img->GetPixelType();
//}

