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
#include "mitkCompareImageFilter.h"
#include "mitkImageAccessByItk.h"

#include <itkTestingComparisonImageFilter.h>

mitk::CompareImageFilter::CompareImageFilter()
{
}


void mitk::CompareImageFilter::GenerateData()
{
  // check inputs

  // check input validity
  if( m_InputImage1->GetDimension() == m_InputImage2->GetDimension() )
  {
    unsigned int dimension = m_InputImage1->GetDimension();

    if( dimension == 2 )
    {
      AccessTwoImagesFixedDimensionByItk( m_InputImage1, m_InputImage2, EstimateValueDifference, 2);
    }
    else if ( dimension == 3 )
    {
      AccessTwoImagesFixedDimensionByItk( m_InputImage1, m_InputImage2, EstimateValueDifference, 3);
    }


  }
}

#include "itkStatisticsImageFilter.h"

template< typename TPixel1, unsigned int VImageDimension1,
          typename TPixel2, unsigned int VImageDimension2>
void mitk::CompareImageFilter::EstimateValueDifference(
                              itk::Image< TPixel1, VImageDimension1>* itkImage1,
                              itk::Image< TPixel2, VImageDimension2>* itkImage2)
{
  typedef itk::Image< TPixel1, VImageDimension1> InputImageType1;
  typedef itk::Image< TPixel2, VImageDimension2> InputImageType2;
  typedef itk::Image< float, VImageDimension> OutputImageType;

  typedef itk::AbsoluteValueDifferenceImageFilter< InputImageType1, InputImageType2, OutputImageType> AbsValueDifferenceFilterType;
  typedef typename AbsValueDifferenceFilterType::Pointer AbsValueDifferenceFilterTypePointer;

  typedef itk::StatisticsImageFilter< OutputImageType > StatisticsImageFilterType;
  typedef typename StatisticsImageFilterType::Pointer StatisticsImageFilterPointer;

  // compute the absolute difference of the two input images
  AbsValueDifferenceFilterTypePointer differenceFilter = AbsValueDifferenceFilterType::New();
  differenceFilter->SetInput1(itkImage1);
  differenceFilter->SetInput2(itkImage2);
  try
  {
    differenceFilter->Update();
  }
  catch( itk::ExceptionObject& e)
  {
    MITK_ERROR << "Catched exception while executing AbsoluteValueDifferenceFilter \n " << e.what();
  }

  // compute statistics over the difference image
  StatisticsImageFilterPointer statisticsFilter = StatisticsImageFilterType::New();
  statisticsFilter->SetInput( differenceFilter->GetOutput() );

  try
  {
    statisticsFilter->Update();
  }
  catch( itk::ExceptionObject& e)
  {
    MITK_ERROR << "Catched exception while executing statistics computation on the difference filter output. \n " << e.what();
  }

  // get the values from the statistics filter
  this->m_MaxValue    = statisticsFilter->GetMaximum();
  this->m_MinValue    = statisticsFilter->GetMinimum();
  this->m_ValueSum = statisticsFilter->GetSum();
  this->m_ValueVariance = statisticsFilter->GetVariance();



}

float mitk::CompareImageFilter::GetMaxValue()
{
  if( Dim == 3)
  //return m_OutputImage->GetMaxValue()
}

void mitk::CompareImageFilter::SetInputImage1(mitk::Image::Pointer image)
{
  this->m_InputImage1 = image;
}

void mitk::CompareImageFilter::SetInputImage2(mitk::Image::Pointer image)
{
  this->m_InputImage2 = image;
}
