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
#include "mitkMultiComponentImageDataComparisonFilter.h"
#include "mitkImageReadAccessor.h"
#include "mitkImagePixelReadAccessor.h"

// other includes
// #include <omp.h>

namespace mitk
{

  MultiComponentImageDataComparisonFilter::MultiComponentImageDataComparisonFilter(): ImageToImageFilter(),
    m_Tolerance(0.0f), m_CompareResult(false), m_CompareDetails(NULL)
  {
    this->SetNumberOfRequiredInputs(2);
  }

  MultiComponentImageDataComparisonFilter::~MultiComponentImageDataComparisonFilter()
  {
  }

  bool MultiComponentImageDataComparisonFilter::GetResult(double threshold)
  {
    if ( !m_CompareResult )
    {
      return false;
    }

    if( m_CompareDetails->m_PixelsWithDifference > threshold )
    {
      return false;
    }

    return true;
  }

  void MultiComponentImageDataComparisonFilter::SetValidImage( const Image *_arg )
  {
    this->SetInput(0, _arg);
  }

  void MultiComponentImageDataComparisonFilter::SetTestImage( const Image *_arg )
  {
    this->SetInput(1, _arg);
  }

  const Image* MultiComponentImageDataComparisonFilter::GetValidImage()
  {
    return this->GetInput(0);
  }

  const Image* MultiComponentImageDataComparisonFilter::GetTestImage()
  {
    return this->GetInput(1);
  }

  void MultiComponentImageDataComparisonFilter::SetCompareFilterResult( CompareFilterResults* results )
  {
    m_CompareDetails = results;
  }

  CompareFilterResults* MultiComponentImageDataComparisonFilter::GetCompareFilterResult()
  {
    return m_CompareDetails;
  }

  void MultiComponentImageDataComparisonFilter::GenerateData()
  {
    // check inputs
    const Image* testInput = this->GetTestImage();
    const Image* validInput = this->GetValidImage();

    // Generally this filter is part of the mitk::Image::Equal() method and only checks the equality of the image data
    // so no further image type comparison is performed!
    // CAVE: If the images differ in a parameter other then the image data, the filter may fail!!

    PixelType type = validInput->GetPixelType();

    if(type.GetComponentType() == itk::ImageIOBase::CHAR)
    {
      CompareMultiComponentImage<char>( testInput, validInput);
    }
    else if (type.GetComponentType() == itk::ImageIOBase::UCHAR)
    {
      CompareMultiComponentImage<unsigned char>( testInput, validInput);
    }
    else if (type.GetComponentType() == itk::ImageIOBase::INT)
    {
      CompareMultiComponentImage<int>( testInput, validInput);
    }
    else if (type.GetComponentType() == itk::ImageIOBase::UINT)
    {
      CompareMultiComponentImage<unsigned int>( testInput, validInput);
    }
    else if (type.GetComponentType() == itk::ImageIOBase::SHORT)
    {
      CompareMultiComponentImage<short>( testInput, validInput);
    }
    else if (type.GetComponentType() == itk::ImageIOBase::USHORT)
    {
      CompareMultiComponentImage<unsigned short>( testInput, validInput);
    }
    else if (type.GetComponentType() == itk::ImageIOBase::LONG)
    {
      CompareMultiComponentImage<long>( testInput, validInput);
    }
    else if (type.GetComponentType() == itk::ImageIOBase::ULONG)
    {
      CompareMultiComponentImage<unsigned long>( testInput, validInput);
    }
    else if (type.GetComponentType() == itk::ImageIOBase::FLOAT)
    {
      CompareMultiComponentImage<float>( testInput, validInput);
    }
    else if (type.GetComponentType() == itk::ImageIOBase::DOUBLE)
    {
      CompareMultiComponentImage<double>( testInput, validInput);
    }
    else
    {
      mitkThrow() << "Pixel component type not supported!";
    }
  }

  template <typename TPixel >
  void mitk::MultiComponentImageDataComparisonFilter::CompareMultiComponentImage( const Image* testImage, const Image* validImage )
  {

    unsigned int noOfTimes = validImage->GetDimension(3);
    unsigned int noOfPixels = validImage->GetDimension(0)*validImage->GetDimension(1)*validImage->GetDimension(2);
    unsigned int noOfComponents = validImage->GetPixelType().GetNumberOfComponents();

    for( unsigned int t = 0; t < noOfTimes; ++t)
    {

      ImageReadAccessor readAccTImage(testImage, testImage->GetVolumeData(t));
      ImageReadAccessor readAccVImage(validImage, validImage->GetVolumeData(t));

      for( unsigned int p = 0; p < noOfPixels*noOfComponents; ++p )
      {
        TPixel vDataItem = static_cast<TPixel*>(const_cast<void*>(readAccVImage.GetData()))[p];
        TPixel tDataItem = static_cast<TPixel*>(const_cast<void*>(readAccTImage.GetData()))[p];

        if( std::abs( static_cast<double>(tDataItem - vDataItem) )>m_Tolerance )
        {
          ++m_CompareDetails->m_PixelsWithDifference;

          m_CompareDetails->m_MaximumDifference = std::max(m_CompareDetails->m_MaximumDifference,
            std::abs( static_cast<double>(tDataItem - vDataItem) ) );

          double min = std::min(m_CompareDetails->m_MinimumDifference,
            std::abs( static_cast<double>(tDataItem - vDataItem) ));

          if(min != 0.0f)                                 // a difference of zero is not a difference!
            m_CompareDetails->m_MinimumDifference = min;

          m_CompareDetails->m_TotalDifference += std::abs(static_cast<double>(tDataItem - vDataItem) );

        }
      }
    }
    if(m_CompareDetails->m_PixelsWithDifference > 0)
    {
      m_CompareDetails->m_MeanDifference = m_CompareDetails->m_TotalDifference / m_CompareDetails->m_PixelsWithDifference;
      m_CompareResult = false;
    }
    else
    {
      m_CompareResult = true;
    }
    m_CompareDetails->m_FilterCompleted = true;
  }

} // end namespace mitk
