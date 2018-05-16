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
#ifndef itkVnlFFT1DRealToComplexConjugateImageFilter_hxx
#define itkVnlFFT1DRealToComplexConjugateImageFilter_hxx

#include "itkVnlFFT1DRealToComplexConjugateImageFilter.h"

#include "itkFFT1DRealToComplexConjugateImageFilter.hxx"
#include "itkImageLinearConstIteratorWithIndex.h"
#include "itkImageLinearIteratorWithIndex.h"
#include "itkIndent.h"
#include "itkMetaDataObject.h"
#include "itkExceptionObject.h"
#include "itkVnlFFTCommon.h"
#include "vnl/algo/vnl_fft_base.h"
#include "vnl/algo/vnl_fft_1d.h"

namespace itk
{

template< typename TInputImage, typename TOutputImage >
void
VnlFFT1DRealToComplexConjugateImageFilter< TInputImage, TOutputImage >
::ThreadedGenerateData( const OutputImageRegionType& outputRegion, ThreadIdType itkNotUsed( threadID ) )
{
  // get pointers to the input and output
  const typename Superclass::InputImageType * inputPtr = this->GetInput();
  typename Superclass::OutputImageType * outputPtr = this->GetOutput();

  const typename Superclass::InputImageType::SizeType & inputSize = inputPtr->GetRequestedRegion().GetSize();

  const unsigned int direction = this->GetDirection();
  unsigned int vectorSize = inputSize[direction];
  if( ! VnlFFTCommon::IsDimensionSizeLegal(vectorSize) )
    {
    itkExceptionMacro("Illegal Array DIM for FFT");
    }


  typedef ImageLinearConstIteratorWithIndex< InputImageType >  InputIteratorType;
  typedef ImageLinearIteratorWithIndex< OutputImageType >      OutputIteratorType;
  InputIteratorType inputIt( inputPtr, outputRegion );
  OutputIteratorType outputIt( outputPtr, outputRegion );

  inputIt.SetDirection( direction );
  outputIt.SetDirection( direction );

  typedef typename TInputImage::PixelType PixelType;
  typedef vcl_complex< PixelType >        ComplexType;
  typedef vnl_vector< ComplexType >       ComplexVectorType;
  ComplexVectorType inputBuffer( vectorSize );
  typename ComplexVectorType::iterator inputBufferIt = inputBuffer.begin();
    // fft is done in-place
  typename ComplexVectorType::iterator outputBufferIt = inputBuffer.begin();
  vnl_fft_1d< PixelType > v1d( vectorSize );

  // for every fft line
  for( inputIt.GoToBegin(), outputIt.GoToBegin();
       !inputIt.IsAtEnd();
       outputIt.NextLine(), inputIt.NextLine() )
    {
    // copy the input line into our buffer
    inputIt.GoToBeginOfLine();
    inputBufferIt = inputBuffer.begin();
    while( !inputIt.IsAtEndOfLine() )
      {
      *inputBufferIt = inputIt.Value();
      ++inputIt;
      ++inputBufferIt;
      }

    // do the transform
    v1d.bwd_transform( inputBuffer );

    // copy the output from the buffer into our line
    outputBufferIt = inputBuffer.begin();
    outputIt.GoToBeginOfLine();
    while( !outputIt.IsAtEndOfLine() )
      {
      outputIt.Set( *outputBufferIt );
      ++outputIt;
      ++outputBufferIt;
      }
    }
}

} // end namespace itk

#endif
