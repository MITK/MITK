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
#include "mitkITKImageImport.h"

#include <itkTestingComparisonImageFilter.h>

mitk::CompareImageFilter::CompareImageFilter()
{
  this->SetNumberOfRequiredInputs(2);
}


void mitk::CompareImageFilter::GenerateData()
{
  // check inputs

  const mitk::Image* input1 = this->GetInput(0);
  const mitk::Image* input2 = this->GetInput(1);

  // check input validity
  if( input1->GetDimension() == input2->GetDimension() )
  {
    AccessByItk_1( input1, EstimateValueDifference, input2);
  }
}

#include "mitkImageCaster.h"

bool mitk::CompareImageFilter::GetResult(size_t threshold)
{
  if (! m_CompareResult)
  {
    return false;
  }

  if( m_CompareDetails.m_PixelsWithDifference > threshold )
  {
    return false;
  }

  return true;
}

template< typename TPixel, unsigned int VImageDimension>
void mitk::CompareImageFilter::EstimateValueDifference(itk::Image< TPixel, VImageDimension>* itkImage1,
                              const mitk::Image* referenceImage)
{

  typedef itk::Image< TPixel, VImageDimension> InputImageType;
  typedef itk::Image< double, VImageDimension > OutputImageType;

  typename InputImageType::Pointer itk_reference = InputImageType::New();

  mitk::CastToItkImage( referenceImage,
                        itk_reference );

  typedef itk::Testing::ComparisonImageFilter< InputImageType, OutputImageType > CompareFilterType;
  typename CompareFilterType::Pointer compare_filter = CompareFilterType::New();
  compare_filter->SetTestInput( itkImage1 );
  compare_filter->SetValidInput( itk_reference );

  try
  {
    compare_filter->Update();
  }
  catch( const itk::ExceptionObject& e)
  {
    m_CompareDetails.m_FilterCompleted = false;
    m_CompareDetails.m_ExceptionMessage = e.what();

    MITK_WARN << e.what();

    m_CompareResult = false;
    return;
  }

  // the filter has completed the calculation
  m_CompareResult = true;
  m_CompareDetails.m_FilterCompleted = true;

  m_CompareDetails.m_MaximumDifference = compare_filter->GetMaximumDifference();
  m_CompareDetails.m_MinimumDifference = compare_filter->GetMinimumDifference();
  m_CompareDetails.m_MeanDifference = compare_filter->GetMeanDifference();
  m_CompareDetails.m_TotalDifference = compare_filter->GetTotalDifference();
  m_CompareDetails.m_PixelsWithDifference = compare_filter->GetNumberOfPixelsWithDifferences();

  mitk::Image::Pointer output = mitk::GrabItkImageMemory( compare_filter->GetOutput() );
  this->SetOutput( MakeNameFromOutputIndex(0), output.GetPointer() );


}
