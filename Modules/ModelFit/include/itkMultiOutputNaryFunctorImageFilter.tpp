/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef itkMultiOutputNaryFunctorImageFilter_tpp
#define itkMultiOutputNaryFunctorImageFilter_tpp

#include <algorithm>
#include <vector>

#include <itkMultiOutputNaryFunctorImageFilter.h>
#include <itkImageRegionIterator.h>

namespace itk
{
  /**
  * Constructor
  */
  template< class TInputImage, class TOutputImage, class TFunction, class TMaskImage >
  MultiOutputNaryFunctorImageFilter< TInputImage, TOutputImage, TFunction, TMaskImage >
    ::MultiOutputNaryFunctorImageFilter()
  {
    // This number will be incremented each time an image
    // is added over the two minimum required
    this->SetNumberOfRequiredInputs(1);

    this->ActualizeOutputs();
  }

  template< class TInputImage, class TOutputImage, class TFunction, class TMaskImage >
  void
    MultiOutputNaryFunctorImageFilter< TInputImage, TOutputImage, TFunction, TMaskImage >
    ::ActualizeOutputs()
  {
    this->SetNumberOfRequiredOutputs(m_Functor.GetNumberOfOutputs());

    for (typename Superclass::DataObjectPointerArraySizeType i = this->GetNumberOfIndexedOutputs(); i< m_Functor.GetNumberOfOutputs(); ++i)
    {
      this->SetNthOutput( i, this->MakeOutput(i) );
    }

    while(this->GetNumberOfIndexedOutputs() > m_Functor.GetNumberOfOutputs())
    {
      this->RemoveOutput(this->GetNumberOfIndexedOutputs()-1);
    }
  };

  /**
  * DynamicThreadedGenerateData Performs the pixel-wise operation
  */
  template< class TInputImage, class TOutputImage, class TFunction, class TMaskImage >
  void
    MultiOutputNaryFunctorImageFilter< TInputImage, TOutputImage, TFunction, TMaskImage >
    ::DynamicThreadedGenerateData(const OutputImageRegionType & outputRegionForThread)
  {
    const unsigned int numberOfInputImages =
      static_cast< unsigned int >( this->GetNumberOfIndexedInputs() );

    const unsigned int numberOfOutputImages =
      static_cast< unsigned int >( this->GetNumberOfIndexedOutputs() );

    typedef ImageRegionConstIterator< TInputImage > ImageRegionConstIteratorType;
    std::vector< ImageRegionConstIteratorType > inputItrVector;
    inputItrVector.reserve(numberOfInputImages);

    typedef ImageRegionIterator< TOutputImage > OutputImageRegionIteratorType;
    std::vector< OutputImageRegionIteratorType > outputItrVector;
    outputItrVector.reserve(numberOfOutputImages);

    //check if mask image is set and generate iterator if mask is valid
    typedef ImageRegionConstIterator< TMaskImage > MaskImageRegionIteratorType;
    MaskImageRegionIteratorType maskIterator;
    const bool hasMask = m_Mask.IsNotNull();

    if (hasMask)
    {
      if (!m_Mask->GetLargestPossibleRegion().IsInside(outputRegionForThread))
      {
        itkExceptionMacro("Mask of filter is set but does not cover region of work unit. Mask region: "<< m_Mask->GetLargestPossibleRegion() <<"Work unit region: "<<outputRegionForThread)
      }
      maskIterator = MaskImageRegionIteratorType(m_Mask, outputRegionForThread);
    }

    // go through the inputs and add iterators for non-null inputs
    for ( unsigned int i = 0; i < numberOfInputImages; ++i )
    {
      InputImagePointer inputPtr =
        dynamic_cast< TInputImage * >( ProcessObject::GetInput(i) );

      if ( inputPtr )
      {
        inputItrVector.push_back( ImageRegionConstIteratorType(inputPtr, outputRegionForThread) );
      }
    }

    // go through the outputs and add iterators for non-null outputs
    for ( unsigned int i = 0; i < numberOfOutputImages; ++i )
    {
      OutputImagePointer outputPtr =
        dynamic_cast< TOutputImage * >( ProcessObject::GetOutput(i) );

      if ( outputPtr )
      {
        outputItrVector.push_back( OutputImageRegionIteratorType(outputPtr, outputRegionForThread) );
      }
    }

    const auto numberOfValidInputImages = inputItrVector.size();
    const auto numberOfValidOutputImages = outputItrVector.size();

    if ( numberOfValidInputImages == 0 || numberOfValidOutputImages == 0 )
    {
      return;
    }

    while ( !(outputItrVector.front().IsAtEnd()) )
    {
      NaryInputArrayType naryInputArray(numberOfValidInputImages);
      NaryOutputArrayType naryOutputArray(numberOfValidOutputImages);

      bool isValid = true;

      if (hasMask)
      {
        isValid = maskIterator.Get() > 0;
        ++maskIterator;
      }

      const typename ImageRegionConstIteratorType::IndexType currentIndex =
        inputItrVector.front().GetIndex();

      typename NaryInputArrayType::iterator arrayInIt = naryInputArray.begin();

      for (auto& inputItr : inputItrVector)
      {
        *arrayInIt++ = inputItr.Get();
        ++inputItr;
      }

      if (isValid)
      {
        naryOutputArray = m_Functor(naryInputArray, currentIndex);

        if (numberOfValidOutputImages != naryOutputArray.size())
        {
          itkExceptionMacro("Error. Number of valid output images do not equal number of outputs required by functor. Number of valid outputs: "<< numberOfValidOutputImages << "; needed output number:" << this->m_Functor.GetNumberOfOutputs());
        }
      }
      else
      {
        std::fill(naryOutputArray.begin(), naryOutputArray.end(), 0.0);
      }

      typename NaryOutputArrayType::iterator arrayOutIt = naryOutputArray.begin();

      for (auto& outputItr : outputItrVector)
      {
        outputItr.Set(*arrayOutIt++);
        ++outputItr;
      }
    }
  }
} // end namespace itk

#endif
