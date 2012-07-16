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

#include "mitkImageAccessByItk.h"
#include "mitkITKImageImport.h"
#include "mitk2DTo3DImageFilter.h"

mitk::2DTo3DImageFilter::BilateralFilter()
  : m_DomainSigma(2.0f), m_RangeSigma(50.0f), m_AutoKernel(true), m_KernelRadius(1u)
{
  //default parameters DomainSigma: 2 , RangeSigma: 50, AutoKernel: true, KernelRadius: 1
}

mitk::BilateralFilter::~BilateralFilter()
{
}

void mitk::BilateralFilter::GenerateData()
{

}

template<typename TPixel, unsigned int VImageDimension>
void mitk::BilateralFilter::ItkImageProcessing( itk::Image<TPixel,VImageDimension>* itkImage )
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
