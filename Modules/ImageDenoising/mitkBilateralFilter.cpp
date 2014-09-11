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

#include "mitkBilateralFilter.h"
#include <itkBilateralImageFilter.h>
#include "mitkImageAccessByItk.h"
#include "mitkImageCast.h"

mitk::BilateralFilter::BilateralFilter()
  : m_DomainSigma(2.0f), m_RangeSigma(50.0f), m_AutoKernel(true), m_KernelRadius(1u)
{
  //default parameters DomainSigma: 2 , RangeSigma: 50, AutoKernel: true, KernelRadius: 1
}

mitk::BilateralFilter::~BilateralFilter()
{
}

void mitk::BilateralFilter::GenerateData()
{
  mitk::Image::ConstPointer inputImage = this->GetInput(0);
  if ( (inputImage->GetDimension() > 4) || (inputImage->GetDimension() < 2) )
  {
    MITK_ERROR << "mitk::BilateralFilter:GenerateData works only with 2D, 2D+t, 3D, 3D+t and 4D images, sorry." << std::endl;
    itkExceptionMacro("mitk::BilateralFilter:GenerateData works only with 2D, 2D+t, 3D, 3D+t and 4D images, sorry.");
    return;
  }
  switch(inputImage->GetDimension())
  {
  case 2:
    {
      AccessFixedDimensionByItk( inputImage.GetPointer(), ItkImageProcessing, 2 ); break;
    }
  case 3:
    {
      AccessFixedDimensionByItk( inputImage.GetPointer(), ItkImageProcessing, 3 ); break;
    }
  case 4:
    {
      AccessFixedDimensionByItk( inputImage.GetPointer(), ItkImageProcessing, 4 ); break;
    }
  default: break;
  }
}

template<typename TPixel, unsigned int VImageDimension>
void mitk::BilateralFilter::ItkImageProcessing( const itk::Image<TPixel,VImageDimension>* itkImage )
{
  //ITK Image type given from the input image
  typedef itk::Image< TPixel, VImageDimension >   ItkImageType;
  //bilateral filter with same type
  typedef itk::BilateralImageFilter<ItkImageType,ItkImageType>        BilateralFilterType;
  typename BilateralFilterType::Pointer bilateralFilter = BilateralFilterType::New();
  bilateralFilter->SetInput(itkImage);
  //set parameters
  if(!m_AutoKernel){
    bilateralFilter->SetAutomaticKernelSize(m_AutoKernel);
    bilateralFilter->SetRadius(m_KernelRadius);
  }
  bilateralFilter->SetDomainSigma(m_DomainSigma);
  bilateralFilter->SetRangeSigma(m_RangeSigma);
  bilateralFilter->UpdateLargestPossibleRegion();
  //get  Pointer to output image
  mitk::Image::Pointer resultImage = this->GetOutput();
  //write into output image
  mitk::CastToMitkImage(bilateralFilter->GetOutput(), resultImage);
}

void mitk::BilateralFilter::GenerateOutputInformation()
{
  mitk::Image::Pointer inputImage  = (mitk::Image*) this->GetInput();
  mitk::Image::Pointer output = this->GetOutput();
  itkDebugMacro(<<"GenerateOutputInformation()");
  if(inputImage.IsNull()) return;
}
