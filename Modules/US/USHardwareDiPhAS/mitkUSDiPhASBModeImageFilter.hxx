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
#ifndef itkPhotoacousticBModeImageFilter_hxx
#define itkPhotoacousticBModeImageFilter_hxx

#include "mitkUSDiPhASBModeImageFilter.h"

#include "itkMetaDataDictionary.h"

#include <algorithm>
#include <sstream>
#include <string>

namespace itk
{

  template < typename TInputImage, typename TOutputImage, typename TComplexImage >
  PhotoacousticBModeImageFilter< TInputImage, TOutputImage, TComplexImage >
    ::PhotoacousticBModeImageFilter()
  {
    m_AnalyticFilter = AnalyticType::New();
    m_ComplexToModulusFilter = ComplexToModulusType::New();
    m_PadFilter = PadType::New();
    m_ROIFilter = ROIType::New();

    m_PadFilter->SetConstant(0.);
    m_ComplexToModulusFilter->SetInput(m_AnalyticFilter->GetOutput());
    m_ROIFilter->SetInput(m_ComplexToModulusFilter->GetOutput());
  }


  template < typename TInputImage, typename TOutputImage, typename TComplexImage >
  void
    PhotoacousticBModeImageFilter< TInputImage, TOutputImage, TComplexImage >
    ::PrintSelf(std::ostream& os, Indent indent) const
  {
    Superclass::PrintSelf(os, indent);
  }


  template < typename TInputImage, typename TOutputImage, typename TComplexImage >
  void
    PhotoacousticBModeImageFilter< TInputImage, TOutputImage, TComplexImage >
    ::GenerateInputRequestedRegion()
  {
    // call the superclass' implementation of this method
    Superclass::GenerateInputRequestedRegion();

    // get pointers to the inputs
    InputImageType * inputPtr = const_cast<InputImageType *> (this->GetInput());
    OutputImageType * outputPtr = this->GetOutput();

    // we need to compute the input requested region (size and start index)
    typedef const typename OutputImageType::SizeType& OutputSizeType;
    OutputSizeType outputRequestedRegionSize =
      outputPtr->GetRequestedRegion().GetSize();
    typedef const typename OutputImageType::IndexType& OutputIndexType;
    OutputIndexType outputRequestedRegionStartIndex =
      outputPtr->GetRequestedRegion().GetIndex();

    //// the regions other than the fft direction are fine
    typename InputImageType::SizeType  inputRequestedRegionSize = outputRequestedRegionSize;
    typename InputImageType::IndexType inputRequestedRegionStartIndex = outputRequestedRegionStartIndex;

    // we but need all of the input in the fft direction
    const unsigned int direction = this->GetDirection();
    const typename InputImageType::SizeType& inputLargeSize =
      inputPtr->GetLargestPossibleRegion().GetSize();
    inputRequestedRegionSize[direction] = inputLargeSize[direction];
    const typename InputImageType::IndexType& inputLargeIndex =
      inputPtr->GetLargestPossibleRegion().GetIndex();
    inputRequestedRegionStartIndex[direction] = inputLargeIndex[direction];

    typename InputImageType::RegionType inputRequestedRegion;
    inputRequestedRegion.SetSize(inputRequestedRegionSize);
    inputRequestedRegion.SetIndex(inputRequestedRegionStartIndex);

    inputPtr->SetRequestedRegion(inputRequestedRegion);
  }


  template < typename TInputImage, typename TOutputImage, typename TComplexImage >
  void
    PhotoacousticBModeImageFilter< TInputImage, TOutputImage, TComplexImage >
    ::EnlargeOutputRequestedRegion(DataObject *output)
  {
    OutputImageType* outputPtr = dynamic_cast< OutputImageType* >(output);

    // we need to enlarge the region in the fft direction to the
    // largest possible in that direction
    typedef const typename OutputImageType::SizeType& ConstOutputSizeType;
    ConstOutputSizeType requestedSize =
      outputPtr->GetRequestedRegion().GetSize();
    ConstOutputSizeType outputLargeSize =
      outputPtr->GetLargestPossibleRegion().GetSize();
    typedef const typename OutputImageType::IndexType& ConstOutputIndexType;
    ConstOutputIndexType requestedIndex =
      outputPtr->GetRequestedRegion().GetIndex();
    ConstOutputIndexType outputLargeIndex =
      outputPtr->GetLargestPossibleRegion().GetIndex();

    typename OutputImageType::SizeType enlargedSize = requestedSize;
    typename OutputImageType::IndexType enlargedIndex = requestedIndex;
    const unsigned int direction = this->GetDirection();
    enlargedSize[direction] = outputLargeSize[direction];
    enlargedIndex[direction] = outputLargeIndex[direction];

    typename OutputImageType::RegionType enlargedRegion;
    enlargedRegion.SetSize(enlargedSize);
    enlargedRegion.SetIndex(enlargedIndex);
    outputPtr->SetRequestedRegion(enlargedRegion);
  }


  template < typename TInputImage, typename TOutputImage, typename TComplexImage >
  void
    PhotoacousticBModeImageFilter< TInputImage, TOutputImage, TComplexImage >
    ::GenerateData()
  {
    this->AllocateOutputs();

    const InputImageType * inputPtr = this->GetInput();
    OutputImageType * outputPtr = this->GetOutput();

    const unsigned int direction = m_AnalyticFilter->GetDirection();
    typename InputImageType::SizeType size = inputPtr->GetLargestPossibleRegion().GetSize();

    // Zero padding.  FFT direction should be factorable by 2 for all FFT
    // implementations to work.
    unsigned int n = size[direction];
    while (n % 2 == 0)
    {
      n /= 2;
    }
    bool doPadding;
    if (n == 1)
    {
      doPadding = false;
    }
    else
    {
      doPadding = true;
    }
    if (doPadding)
    {
      n = size[direction];
      unsigned int newSizeDirection = 1;
      while (newSizeDirection < n)
      {
        newSizeDirection *= 2;
      }
      typename InputImageType::SizeType padSize;
      padSize.Fill(0);
      padSize[direction] = newSizeDirection - size[direction];
      size[direction] = newSizeDirection;
      m_PadFilter->SetPadUpperBound(padSize);
      m_PadFilter->SetInput(inputPtr);
      m_AnalyticFilter->SetInput(m_PadFilter->GetOutput());
      m_ROIFilter->SetReferenceImage(inputPtr);
      m_ROIFilter->SetInput(m_ComplexToModulusFilter->GetOutput());
      m_ROIFilter->GraftOutput(outputPtr);
      m_ROIFilter->Update();
      this->GraftOutput(m_ROIFilter->GetOutput());
    }
    else // padding is not required
    {
      m_AnalyticFilter->SetInput(inputPtr);
      m_ComplexToModulusFilter->GraftOutput(outputPtr);
      m_ComplexToModulusFilter->Update();
      this->GraftOutput(m_ComplexToModulusFilter->GetOutput());
    }

  }

} // end namespace itk

#endif