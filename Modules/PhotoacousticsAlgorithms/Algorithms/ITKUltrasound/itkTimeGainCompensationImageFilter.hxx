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
/*=========================================================================
 *
 *  Copyright Insight Software Consortium
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0.txt
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *=========================================================================*/
#ifndef itkTimeGainCompensationImageFilter_hxx
#define itkTimeGainCompensationImageFilter_hxx

#include "itkTimeGainCompensationImageFilter.h"

#include "itkImageScanlineIterator.h"
#include "itkImageScanlineConstIterator.h"
#include "itkArray.h"

namespace itk
{

template< typename TInputImage, typename TOutputImage >
TimeGainCompensationImageFilter< TInputImage, TOutputImage >
::TimeGainCompensationImageFilter():
  m_Gain( 2, 2 )
{
  m_Gain(0, 0) = NumericTraits< double >::min();
  m_Gain(0, 1) = NumericTraits< double >::OneValue();
  m_Gain(1, 0) = NumericTraits< double >::max();
  m_Gain(1, 1) = NumericTraits< double >::OneValue();
}


template< typename TInputImage, typename TOutputImage >
void
TimeGainCompensationImageFilter< TInputImage, TOutputImage >
::PrintSelf( std::ostream & os, Indent indent ) const
{
  Superclass::PrintSelf( os, indent );

  os << indent << "Gain:" << std::endl;
  for( unsigned int ii = 0; ii < m_Gain.rows(); ++ii )
    {
    os << indent.GetNextIndent() << "[" << m_Gain( ii, 0 ) << ", " << m_Gain( ii, 1 ) << "]" << std::endl;
    }
}


template< typename TInputImage, typename TOutputImage >
void
TimeGainCompensationImageFilter< TInputImage, TOutputImage >
::BeforeThreadedGenerateData()
{
  const GainType & gain = this->GetGain();
  if( gain.cols() != 2 )
    {
    itkExceptionMacro( "Gain should have two columns." );
    }
  if( gain.rows() < 2 )
    {
    itkExceptionMacro( "Insufficient depths specified in Gain." );
    }
  double depth = gain( 0, 0 );
  for( unsigned int ii = 1; ii < gain.rows(); ++ii )
    {
    if( gain( ii, 0 ) <= depth )
      {
      itkExceptionMacro( "Gain depths must be strictly increasing." );
      }
    depth = gain( ii, 0 );
    }
}


template< typename TInputImage, typename TOutputImage >
void
TimeGainCompensationImageFilter< TInputImage, TOutputImage >
::ThreadedGenerateData( const OutputImageRegionType & outputRegionForThread, ThreadIdType itkNotUsed( threadId ) )
{
  const InputImageType * inputImage = this->GetInput();
  OutputImageType * outputImage = this->GetOutput();

  // Compute the line gain once.
  const GainType & gain = this->GetGain();
  double pieceStart = gain( 0, 0 );
  double pieceEnd = gain( 1, 0 );
  double gainStart = gain( 0, 1 );
  double gainEnd = gain( 1, 1 );
  SizeValueType gainSegment = 1;

  typedef Array< double > LineGainType;
  const SizeValueType lineGainSize = outputRegionForThread.GetSize()[0];
  const typename InputImageType::RegionType & inputRegion = inputImage->GetLargestPossibleRegion();
  const IndexValueType imageStartIndex = inputRegion.GetIndex()[0];
  const typename InputImageType::PointType origin = inputImage->GetOrigin();
  const SpacePrecisionType pixelSpacing = inputImage->GetSpacing()[0];
  IndexValueType indexOffset = outputRegionForThread.GetIndex()[0] - imageStartIndex;
  LineGainType lineGain( lineGainSize );
  for( SizeValueType lineGainIndex = 0; lineGainIndex < lineGainSize; ++lineGainIndex )
    {
    const SpacePrecisionType pixelLocation = origin[0] + pixelSpacing * indexOffset;
    if( pixelLocation <= pieceStart )
      {
      lineGain[lineGainIndex] = gainStart;
      }
    else if( pixelLocation > pieceEnd )
      {
      if( gainSegment >= gain.rows() - 1 )
        {
        lineGain[lineGainIndex] = gainEnd;
        }
      else
        {
        ++gainSegment;
        pieceStart = gain( gainSegment - 1, 0 );
        pieceEnd = gain( gainSegment, 0 );
        gainStart = gain( gainSegment - 1, 1 );
        gainEnd = gain( gainSegment, 1 );

        const SpacePrecisionType offset = static_cast< SpacePrecisionType >( pixelLocation - pieceStart );
        lineGain[lineGainIndex] = offset * ( gainEnd - gainStart ) / ( pieceEnd - pieceStart ) + gainStart;
        }
      }
    else
      {
      const SpacePrecisionType offset = static_cast< SpacePrecisionType >( pixelLocation - pieceStart );
      lineGain[lineGainIndex] = offset * ( gainEnd - gainStart ) / ( pieceEnd - pieceStart ) + gainStart;
      }
    ++indexOffset;
    }

  typedef ImageScanlineConstIterator< InputImageType > InputIteratorType;
  InputIteratorType inputIt( inputImage, outputRegionForThread );

  typedef ImageScanlineIterator< OutputImageType > OutputIteratorType;
  OutputIteratorType outputIt( outputImage, outputRegionForThread );

  for( inputIt.GoToBegin(), outputIt.GoToBegin();
       !outputIt.IsAtEnd();
       inputIt.NextLine(), outputIt.NextLine() )
    {
    inputIt.GoToBeginOfLine();
    outputIt.GoToBeginOfLine();
    SizeValueType lineGainIndex = 0;
    while( ! outputIt.IsAtEndOfLine() )
      {
      outputIt.Set( inputIt.Value() * lineGain[lineGainIndex] );
      ++inputIt;
      ++outputIt;
      ++lineGainIndex;
      }
    }
}

} // end namespace itk

#endif // itkTimeGainCompensationImageFilter_hxx
