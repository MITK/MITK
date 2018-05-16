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
#ifndef itkSpectra1DSupportWindowImageFilter_hxx
#define itkSpectra1DSupportWindowImageFilter_hxx

#include "itkSpectra1DSupportWindowImageFilter.h"
#include "itkImageRegionIterator.h"
#include "itkImageLinearConstIteratorWithIndex.h"
#include "itkMetaDataObject.h"
#include "itkImageScanlineIterator.h"

namespace itk
{

template< typename TInputImage >
Spectra1DSupportWindowImageFilter< TInputImage >
::Spectra1DSupportWindowImageFilter():
  m_FFT1DSize( 32 ),
  m_Step( 1 )
{
}


template< typename TInputImage >
void
Spectra1DSupportWindowImageFilter< TInputImage >
::GenerateOutputInformation()
{
  Superclass::GenerateOutputInformation();

  OutputImageType * output = this->GetOutput();
  const InputImageType * input = this->GetInput();

  OutputImageRegionType outputRegion = input->GetLargestPossibleRegion();
  typename OutputImageType::SizeType outputSize = outputRegion.GetSize();
  outputSize[0] /= this->GetStep();
  outputRegion.SetSize( outputSize );
  output->SetLargestPossibleRegion( outputRegion );

  typename OutputImageType::SpacingType outputSpacing = input->GetSpacing();
  outputSpacing[0] *= this->GetStep();
  output->SetSpacing( outputSpacing );
}


template< typename TInputImage >
void
Spectra1DSupportWindowImageFilter< TInputImage >
::ThreadedGenerateData( const OutputImageRegionType & outputRegionForThread, ThreadIdType itkNotUsed( threadId ) )
{
  OutputImageType * output = this->GetOutput();
  const InputImageType * input = this->GetInput();

  const OutputImageRegionType outputLargestRegion = output->GetLargestPossibleRegion();
  typedef typename OutputImageType::IndexType IndexType;
  const IndexType largestIndexStart = outputLargestRegion.GetIndex();
  IndexType largestIndexStop = largestIndexStart + outputLargestRegion.GetSize();
  for( unsigned int dim = 0; dim < ImageDimension; ++dim )
    {
    largestIndexStop[dim] -= 1;
    }

  typedef ImageLinearConstIteratorWithIndex< InputImageType > InputIteratorType;
  InputIteratorType inputIt( input, outputRegionForThread );
  typedef ImageScanlineIterator< OutputImageType > OutputIteratorType;
  OutputIteratorType outputIt( output, outputRegionForThread );
  const FFT1DSizeType fftSize = this->GetFFT1DSize();
  const SizeValueType sampleStep = this->GetStep();
  if( outputLargestRegion.GetSize()[0] < fftSize )
    {
    itkExceptionMacro( "Insufficient size in the FFT direction." );
    }
  for( inputIt.GoToBegin(), outputIt.GoToBegin(); !outputIt.IsAtEnd(); )
    {
    while( !outputIt.IsAtEndOfLine() )
      {
      OutputPixelType & supportWindow = outputIt.Value();
      supportWindow.clear();

      const IndexType inputIndex = inputIt.GetIndex();

      IndexType lineIndex;
      lineIndex[0] = inputIndex[0] - fftSize / 2;
      if( lineIndex[0] < largestIndexStart[0] )
        {
        lineIndex[0] = largestIndexStart[0];
        }

      if( lineIndex[0] + fftSize > largestIndexStop[0] )
        {
        lineIndex[0] = largestIndexStop[0] - fftSize;
        }

      const IndexValueType sideLines = static_cast< IndexValueType >( inputIt.Get() );
      for( IndexValueType line = inputIndex[1] - sideLines;
           line < inputIndex[1] + sideLines;
           ++line )
        {
        if( line < largestIndexStart[1] || line > largestIndexStop[1] )
          {
          continue;
          }
        lineIndex[1] = line;
        supportWindow.push_back( lineIndex );
        }
      for( SizeValueType ii = 0; ii < sampleStep; ++ii )
        {
        ++inputIt;
        }
      ++outputIt;
      }
      inputIt.NextLine();
      outputIt.NextLine();
    }
}


template< typename TInputImage >
void
Spectra1DSupportWindowImageFilter< TInputImage >
::AfterThreadedGenerateData()
{
  OutputImageType * output = this->GetOutput();
  MetaDataDictionary & dict = output->GetMetaDataDictionary();
  EncapsulateMetaData< FFT1DSizeType >( dict, "FFT1DSize", this->GetFFT1DSize() );
}


template< typename TInputImage >
void
Spectra1DSupportWindowImageFilter< TInputImage >
::PrintSelf( std::ostream & os, Indent indent ) const
{
  Superclass::PrintSelf( os, indent );

  os << indent << "FFT1DSize: " << this->GetFFT1DSize() << std::endl;
  os << indent << "Step: " << this->GetStep() << std::endl;
}

} // end namespace itk

#endif
