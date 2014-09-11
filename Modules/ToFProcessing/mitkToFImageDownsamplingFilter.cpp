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

#include "mitkToFImageDownsamplingFilter.h"
#include <itkResampleImageFilter.h>
#include <mitkImageCast.h>
#include <mitkImageAccessByItk.h>
#include <itkImageFileWriter.h>
#include <itkIdentityTransform.h>
#include <itkLinearInterpolateImageFunction.h>
#include <itkNearestNeighborInterpolateImageFunction.h>

mitk::ToFImageDownsamplingFilter::ToFImageDownsamplingFilter():
m_ResampledX(100), m_ResampledY(100),m_ResampledZ(1)
{
}

mitk::ToFImageDownsamplingFilter::~ToFImageDownsamplingFilter()
{
}





void mitk::ToFImageDownsamplingFilter::GenerateData()
{
  // set input image

  mitk::Image::ConstPointer inputImage = this->GetInput(0) ;
  if ( (inputImage->GetDimension() > 3) || (inputImage->GetDimension() < 2) )
  {
    MITK_ERROR << "mitk::TofImageDownsamplingFilter:GenerateData works only with 2D and 3D images, sorry." << std::endl;
    itkExceptionMacro("mitk::TofImageDownsamplingFilter:GenerateData works only with 2D and 3D images, sorry.");
    return;
  }

  if ( (inputImage->GetDimension(0)<m_ResampledX) || (inputImage->GetDimension(1)<m_ResampledY) || (inputImage->GetDimension(2)<m_ResampledZ) )
  {
    MITK_ERROR << "mitk::TofImageDownsamplingFilter:GenerateData only downsamples. Your requested dimensions exceed the original image dimensions." << std::endl;
    itkExceptionMacro("mitk::TofImageDownsamplingFilter:GenerateData only downsamples. Your requested dimensions exceed the original image dimensions.");
    return;
  }

  if ( (m_ResampledX < 1) || (m_ResampledY < 1)|| (m_ResampledZ < 1) )
  {
    MITK_ERROR << "mitk::TofImageDownsamplingFilter:GenerateData works only for positive input dimensions " << std::endl;
    itkExceptionMacro("mitk::TofImageDownsamplingFilter:GenerateData works only for positive input dimensions");
    return;
  }

  switch(inputImage->GetDimension())
  {
  case 2:
    {
      AccessFixedDimensionByItk( inputImage.GetPointer(), ItkImageResampling, 2 ); break;
    }
  case 3:
    {
      AccessFixedDimensionByItk( inputImage.GetPointer(), ItkImageResampling, 3 ); break;
    }

  default: break;
  }


}

template<typename TPixel, unsigned int VImageDimension>
void mitk::ToFImageDownsamplingFilter::ItkImageResampling( const itk::Image<TPixel,VImageDimension>* itkImage )
{
  // declare typdef for itk image from input mitk image
 typedef itk::Image< TPixel, VImageDimension >   ItkImageType;

  //declare itk filter related typedefs (transform type, interpolater, and size type)
  typedef itk::ResampleImageFilter<ItkImageType,ItkImageType>    ResamplerFilterType;
  typedef itk::IdentityTransform<double, VImageDimension> TransformType;
  typedef itk::NearestNeighborInterpolateImageFunction<ItkImageType, double > InterpolatorType;
  typedef typename ItkImageType::SizeType::SizeValueType SizeValueType;

  //instantiate filter related parameters
  typename ResamplerFilterType::Pointer resampler = ResamplerFilterType::New();
  typename TransformType::Pointer transform = TransformType::New();
  typename InterpolatorType::Pointer interpolator = InterpolatorType::New();

  // establish size for downsampled image ( the result of this filter)
  typename ItkImageType::SizeType inputSize = itkImage->GetLargestPossibleRegion().GetSize();
  typename ItkImageType::SizeType size;

  size[0] = static_cast< SizeValueType >( m_ResampledX );
  size[1] = static_cast< SizeValueType >( m_ResampledY );
  size[2] = static_cast< SizeValueType >( m_ResampledZ );

  //establish spacing for new downsampled image ( resulting image)
  const typename ItkImageType::SpacingType& inputSpacing = itkImage->GetSpacing();
  typename ItkImageType::SpacingType outputSpacing;

  outputSpacing[0] = inputSpacing[0] * ( inputSize[0]/ m_ResampledX );
  outputSpacing[1] = inputSpacing[1] * ( inputSize[1]/ m_ResampledY );
  outputSpacing[2] = inputSpacing[2] * ( inputSize[2]/ m_ResampledZ );

  mitk::Vector3D mitkspacing;
  mitkspacing[0] = outputSpacing[0];
  mitkspacing[1] = outputSpacing[1];
  mitkspacing[2] = outputSpacing[2];

  mitk::Point3D mitkorig;
  mitkorig[0] = itkImage->GetOrigin()[0];
  mitkorig[1] = itkImage->GetOrigin()[1];
  mitkorig[2] = 0.0;

  // set filter parameters and update
  transform->SetIdentity();
  resampler->SetTransform(transform);
  resampler->SetInterpolator(interpolator);
  resampler->SetOutputSpacing(outputSpacing);
  resampler->SetOutputOrigin(itkImage->GetOrigin());
  resampler->SetSize(size);
  resampler->SetInput(itkImage);
  resampler->UpdateLargestPossibleRegion();

  // Create mitk container for resulting image
  mitk::Image::Pointer resultImage = ImageToImageFilter::GetOutput();
  resultImage->SetSpacing(mitkspacing);
  resultImage->SetOrigin(mitkorig);

  // Cast itk image to mitk image
  mitk::CastToMitkImage(resampler->GetOutput(), resultImage);
}
