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
#include "mitkMultiChannelImageDataComparisonFilter.h"
#include "mitkImageReadAccessor.h"
#include "mitkImagePixelReadAccessor.h"

// other includes
#include <omp.h>

namespace mitk
{

  MultiChannelImageDataComparisonFilter::MultiChannelImageDataComparisonFilter(): ImageToImageFilter(),
    m_Tolerance(0.0f), m_CompareResult(false), m_CompareDetails(NULL)
  {
    this->SetNumberOfRequiredInputs(2);
  }

  MultiChannelImageDataComparisonFilter::~MultiChannelImageDataComparisonFilter()
  {
  }

  bool MultiChannelImageDataComparisonFilter::GetResult(double threshold)
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

  void MultiChannelImageDataComparisonFilter::SetValidImage( const Image *_arg )
  {
    this->SetInput(0, _arg);
  }

  void MultiChannelImageDataComparisonFilter::SetTestImage( const Image *_arg )
  {
    this->SetInput(1, _arg);
  }

  const Image* MultiChannelImageDataComparisonFilter::GetValidImage()
  {
    return this->GetInput(0);
  }

  const Image* MultiChannelImageDataComparisonFilter::GetTestImage()
  {
    return this->GetInput(1);
  }

  void MultiChannelImageDataComparisonFilter::SetCompareFilterResult( CompareFilterResults* results )
  {
    m_CompareDetails = results;
  }

  CompareFilterResults* MultiChannelImageDataComparisonFilter::GetCompareFilterResult()
  {
    return m_CompareDetails;
  }

  void MultiChannelImageDataComparisonFilter::GenerateData()
  {
    // check inputs
    const Image* testInput = this->GetTestImage();
    const Image* validInput = this->GetValidImage();


    PixelType type = validInput->GetPixelType();

    if(type.GetComponentType() == itk::ImageIOBase::CHAR)
    {
      CompareMultiChannelImage<char>( testInput, validInput);
    }
    else if (type.GetComponentType() == itk::ImageIOBase::UCHAR)
    {
      CompareMultiChannelImage<unsigned char>( testInput, validInput);
    }
    else if (type.GetComponentType() == itk::ImageIOBase::INT)
    {
      CompareMultiChannelImage<int>( testInput, validInput);
    }
    else if (type.GetComponentType() == itk::ImageIOBase::UINT)
    {
      CompareMultiChannelImage<unsigned int>( testInput, validInput);
    }
    else if (type.GetComponentType() == itk::ImageIOBase::SHORT)
    {
      CompareMultiChannelImage<short>( testInput, validInput);
    }
    else if (type.GetComponentType() == itk::ImageIOBase::USHORT)
    {
      CompareMultiChannelImage<unsigned short>( testInput, validInput);
    }
    else if (type.GetComponentType() == itk::ImageIOBase::LONG)
    {
      CompareMultiChannelImage<long>( testInput, validInput);
    }
    else if (type.GetComponentType() == itk::ImageIOBase::ULONG)
    {
      CompareMultiChannelImage<unsigned long>( testInput, validInput);
    }
    else if (type.GetComponentType() == itk::ImageIOBase::FLOAT)
    {
      CompareMultiChannelImage<float>( testInput, validInput);
    }
    else if (type.GetComponentType() == itk::ImageIOBase::DOUBLE)
    {
      CompareMultiChannelImage<double>( testInput, validInput);
    }
    else
    {
    }


    // Generally this filter is part of the mitk::Image::Equal() method and only checks the equality of the image data
    // so no further image type comparison is performed!
    // CAVE: If the images differ in a parameter other then the image data, the filter may fail!!

  }

  template <typename TPixel >
  void mitk::MultiChannelImageDataComparisonFilter::CompareMultiChannelImage( const Image* testImage, const Image* validImage )
  {

    unsigned int noOfTimes = validImage->GetDimension(3);
    unsigned int noOfPixels = validImage->GetDimension(0)*validImage->GetDimension(1)*validImage->GetDimension(2);
    unsigned int noOfComponents = validImage->GetPixelType().GetNumberOfComponents();

    for( int t = 0; t < noOfTimes; ++t)
    {

      ImageReadAccessor readAccTImage(const_cast<Image*>(testImage), const_cast<Image*>(testImage)->GetVolumeData(t));
      ImageReadAccessor readAccVImage(const_cast<Image*>(validImage), const_cast<Image*>(validImage)->GetVolumeData(t));

     // #pragma omp parallel for
      for( int p = 0; p < noOfPixels*noOfComponents; ++p )
      {
        TPixel vDataItem = static_cast<TPixel*>(const_cast<void*>(readAccVImage.GetData()))[p];
        TPixel tDataItem = static_cast<TPixel*>(const_cast<void*>(readAccTImage.GetData()))[p];

        if( tDataItem != vDataItem )
        {
          ++m_CompareDetails->m_PixelsWithDifference;

          m_CompareDetails->m_MaximumDifference = std::max(m_CompareDetails->m_MaximumDifference,
            std::abs( static_cast<double>(tDataItem - vDataItem) ) );

          m_CompareDetails->m_MinimumDifference = std::min(m_CompareDetails->m_MinimumDifference,
            std::abs( static_cast<double>(tDataItem - vDataItem) ));

          m_CompareDetails->m_TotalDifference += std::abs(static_cast<double>(tDataItem - vDataItem) );

        }
      }
    }
    m_CompareDetails->m_MeanDifference = m_CompareDetails->m_TotalDifference / m_CompareDetails->m_PixelsWithDifference;
    m_CompareDetails->m_PixelsWithDifference /= noOfPixels*noOfComponents*noOfTimes;

  }

} // end namespace mitk