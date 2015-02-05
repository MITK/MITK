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

// mitk includes
#include "mitkCompareImageDataFilter.h"
#include "mitkImageAccessByItk.h"
#include "mitkITKImageImport.h"
#include "mitkImageCaster.h"
#include "mitkMultiComponentImageDataComparisonFilter.h"

// itk includes
#include <itkTestingComparisonImageFilter.h>

mitk::CompareImageDataFilter::CompareImageDataFilter()
    : m_Tolerance(0.0)
{
  this->SetNumberOfRequiredInputs(2);

  this->ResetCompareResultsToInitial();
}

void mitk::CompareImageDataFilter::ResetCompareResultsToInitial()
{
  m_CompareDetails.m_MaximumDifference = 0.0f;
  m_CompareDetails.m_MinimumDifference = itk::NumericTraits< double >::max();
  m_CompareDetails.m_MeanDifference = 0.0f;
  m_CompareDetails.m_TotalDifference = 0.0f;
  m_CompareDetails.m_PixelsWithDifference = 0;
  m_CompareDetails.m_FilterCompleted = false;
  m_CompareDetails.m_ExceptionMessage = "";
}

void mitk::CompareImageDataFilter::GenerateData()
{
  // check inputs

  const mitk::Image* input1 = this->GetInput(0);
  const mitk::Image* input2 = this->GetInput(1);

  //   Generally this filter is part of the mitk::Image::Equal() method and only checks the equality of the image data
  //   so no further image type comparison is performed!
  //   CAVE: If the images differ in a parameter other then the image data, the filter may fail!!

  // check what number of components the inputs have
  if(input1->GetPixelType().GetNumberOfComponents() == 1 &&
    input2->GetPixelType().GetNumberOfComponents() == 1)
  {
    AccessByItk_1( input1, EstimateValueDifference, input2);
  }
  else if(input1->GetPixelType().GetNumberOfComponents() > 1 &&
    input2->GetPixelType().GetNumberOfComponents() > 1 )
  {
    this->ResetCompareResultsToInitial();

    MultiComponentImageDataComparisonFilter::Pointer mcComparator = MultiComponentImageDataComparisonFilter::New();
    mcComparator->SetTestImage(input1);
    mcComparator->SetValidImage(input2);
    mcComparator->SetCompareFilterResult( &m_CompareDetails);
    mcComparator->SetTolerance(m_Tolerance);
    mcComparator->Update();

    m_CompareResult = mcComparator->GetResult();
  }
}

bool mitk::CompareImageDataFilter::GetResult(size_t threshold)
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
void mitk::CompareImageDataFilter::EstimateValueDifference(const itk::Image< TPixel, VImageDimension>* itkImage1,
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
  compare_filter->SetDifferenceThreshold( m_Tolerance );

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
