/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef __itkMultiOutputNaryFunctorImageFilter_hxx
#define __itkMultiOutputNaryFunctorImageFilter_hxx

#include "itkMultiOutputNaryFunctorImageFilter.h"
#include "itkImageRegionIterator.h"
#include "itkProgressReporter.h"

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
  * ThreadedGenerateData Performs the pixel-wise addition
  */
  template< class TInputImage, class TOutputImage, class TFunction, class TMaskImage >
  void
    MultiOutputNaryFunctorImageFilter< TInputImage, TOutputImage, TFunction, TMaskImage >
    ::ThreadedGenerateData(const OutputImageRegionType & outputRegionForThread,
    ThreadIdType threadId)
  {
    ProgressReporter progress( this, threadId,
      outputRegionForThread.GetNumberOfPixels() );

    const unsigned int numberOfInputImages =
      static_cast< unsigned int >( this->GetNumberOfIndexedInputs() );

    const unsigned int numberOfOutputImages =
      static_cast< unsigned int >( this->GetNumberOfIndexedOutputs() );

    typedef ImageRegionConstIterator< TInputImage > ImageRegionConstIteratorType;
    std::vector< ImageRegionConstIteratorType * > inputItrVector;
    inputItrVector.reserve(numberOfInputImages);

    typedef ImageRegionIterator< TOutputImage > OutputImageRegionIteratorType;
    std::vector< OutputImageRegionIteratorType * > outputItrVector;
    outputItrVector.reserve(numberOfOutputImages);

    //check if mask image is set and generate iterator if mask is valid
    typedef ImageRegionConstIterator< TMaskImage > MaskImageRegionIteratorType;
    MaskImageRegionIteratorType* pMaskIterator = nullptr;

    if (m_Mask.IsNotNull())
    {
      if (!m_Mask->GetLargestPossibleRegion().IsInside(outputRegionForThread))
      {
        itkExceptionMacro("Mask of filter is set but does not cover region of thread. Mask region: "<< m_Mask->GetLargestPossibleRegion() <<"Thread region: "<<outputRegionForThread)
      }
      pMaskIterator = new MaskImageRegionIteratorType(m_Mask,outputRegionForThread);
    }

    // go through the inputs and add iterators for non-null inputs
    for ( unsigned int i = 0; i < numberOfInputImages; ++i )
    {
      InputImagePointer inputPtr =
        dynamic_cast< TInputImage * >( ProcessObject::GetInput(i) );

      if ( inputPtr )
      {
        inputItrVector.push_back( new ImageRegionConstIteratorType(inputPtr, outputRegionForThread) );
      }

    }

    // go through the outputs and add iterators for non-null outputs
    for ( unsigned int i = 0; i < numberOfOutputImages; ++i )
    {
      OutputImagePointer outputPtr =
        dynamic_cast< TOutputImage * >( ProcessObject::GetOutput(i) );

      if ( outputPtr )
      {
        outputItrVector.push_back( new OutputImageRegionIteratorType(outputPtr, outputRegionForThread) );
      }
    }

    typename std::vector< ImageRegionConstIteratorType * >::iterator regionInputIterators;
    const typename std::vector< ImageRegionConstIteratorType * >::const_iterator regionInputItEnd =
      inputItrVector.end();

    typename std::vector< OutputImageRegionIteratorType * >::iterator regionOutputIterators;
    const typename std::vector< OutputImageRegionIteratorType * >::const_iterator regionOutputItEnd =
      outputItrVector.end();

    const unsigned int numberOfValidInputImages = inputItrVector.size();
    const unsigned int numberOfValidOutputImages = outputItrVector.size();

    if ( (numberOfValidInputImages != 0) && ( numberOfValidOutputImages != 0))
    {
      try
      {

        while ( !(outputItrVector.front()->IsAtEnd()) )
        {
          typename NaryInputArrayType::iterator arrayInIt;
          typename NaryOutputArrayType::iterator arrayOutIt;
          NaryInputArrayType naryInputArray(numberOfValidInputImages);
          NaryOutputArrayType naryOutputArray(numberOfValidOutputImages);

          bool isValid = true;

          if (pMaskIterator)
          {
            isValid = pMaskIterator->Get() > 0;
            ++(*pMaskIterator);
          }

          arrayInIt = naryInputArray.begin();
          regionInputIterators = inputItrVector.begin();

          typename ImageRegionConstIteratorType::IndexType currentIndex;
          if(regionInputIterators != regionInputItEnd)
          {
            currentIndex = ( *regionInputIterators )->GetIndex();
          }

          while ( regionInputIterators != regionInputItEnd )
          {
            *arrayInIt++ = ( *regionInputIterators )->Get();
            ++( *( *regionInputIterators ) );
            ++regionInputIterators;
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
            for (typename NaryOutputArrayType::iterator pos = naryOutputArray.begin(); pos!= naryOutputArray.end(); ++pos)
            {
                *pos = 0.0;
            }
          }

          arrayOutIt = naryOutputArray.begin();
          regionOutputIterators = outputItrVector.begin();
          while ( regionOutputIterators != regionOutputItEnd )
          {
            ( *regionOutputIterators )->Set(*arrayOutIt++);
            ++( *( *regionOutputIterators ) );
            ++regionOutputIterators;
          }

          progress.CompletedPixel();
        }
      }
      catch(...)
      {
        // Free memory in case of exceptions
        regionInputIterators = inputItrVector.begin();
        while ( regionInputIterators != regionInputItEnd )
        {
          delete ( *regionInputIterators++ );
        }

        regionOutputIterators = outputItrVector.begin();
        while ( regionOutputIterators != regionOutputItEnd )
        {
          delete ( *regionOutputIterators++ );
        }

        delete pMaskIterator;
        throw;
      }
    }

    // Free memory regulary
    regionInputIterators = inputItrVector.begin();
    while ( regionInputIterators != regionInputItEnd )
    {
      delete ( *regionInputIterators++ );
    }

    regionOutputIterators = outputItrVector.begin();
    while ( regionOutputIterators != regionOutputItEnd )
    {
      delete ( *regionOutputIterators++ );
    }

    delete pMaskIterator;
  }
} // end namespace itk

#endif
