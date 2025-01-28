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

#ifndef __itkIndexedUnaryFunctorImageFilter_tpp
#define __itkIndexedUnaryFunctorImageFilter_tpp

#include "itkIndexedUnaryFunctorImageFilter.h"
#include "itkImageRegionConstIteratorWithIndex.h"
#include "itkImageRegionIterator.h"
#include "itkProgressReporter.h"

namespace itk
{

  template< typename TInputImage, typename TOutputImage, typename TFunction  >
  IndexedUnaryFunctorImageFilter< TInputImage, TOutputImage, TFunction >
    ::IndexedUnaryFunctorImageFilter()
  {
    this->SetNumberOfRequiredInputs(1);
    this->InPlaceOff();
  }


  template< typename TInputImage, typename TOutputImage, typename TFunction >
  void
    IndexedUnaryFunctorImageFilter< TInputImage, TOutputImage, TFunction >
    ::GenerateOutputInformation()
  {
    // do not call the superclass' implementation of this method since
    // this filter allows the input the output to be of different dimensions

    // get pointers to the input and output
    typename Superclass::OutputImagePointer outputPtr = this->GetOutput();
    typename Superclass::InputImageConstPointer inputPtr = this->GetInput();

    if (!outputPtr || !inputPtr)
    {
      return;
    }

    // Set the output image largest possible region.  Use a RegionCopier
    // so that the input and output images can be different dimensions.
    OutputImageRegionType outputLargestPossibleRegion;
    this->CallCopyInputRegionToOutputRegion(outputLargestPossibleRegion,
      inputPtr->GetLargestPossibleRegion());
    outputPtr->SetLargestPossibleRegion(outputLargestPossibleRegion);

    // Set the output spacing and origin
    const ImageBase< Superclass::InputImageDimension > *phyData;

    phyData =
      dynamic_cast< const ImageBase< Superclass::InputImageDimension > * >(this->GetInput());

    if (phyData)
    {
      // Copy what we can from the image from spacing and origin of the input
      // This logic needs to be augmented with logic that select which
      // dimensions to copy
      unsigned int i, j;
      const typename InputImageType::SpacingType &
        inputSpacing = inputPtr->GetSpacing();
      const typename InputImageType::PointType &
        inputOrigin = inputPtr->GetOrigin();
      const typename InputImageType::DirectionType &
        inputDirection = inputPtr->GetDirection();

      typename OutputImageType::SpacingType outputSpacing;
      typename OutputImageType::PointType outputOrigin;
      typename OutputImageType::DirectionType outputDirection;

      // copy the input to the output and fill the rest of the
      // output with zeros.
      for (i = 0; i < Superclass::InputImageDimension; ++i)
      {
        outputSpacing[i] = inputSpacing[i];
        outputOrigin[i] = inputOrigin[i];
        for (j = 0; j < Superclass::OutputImageDimension; j++)
        {
          if (j < Superclass::InputImageDimension)
          {
            outputDirection[j][i] = inputDirection[j][i];
          }
          else
          {
            outputDirection[j][i] = 0.0;
          }
        }
      }
      for (; i < Superclass::OutputImageDimension; ++i)
      {
        outputSpacing[i] = 1.0;
        outputOrigin[i] = 0.0;
        for (j = 0; j < Superclass::OutputImageDimension; j++)
        {
          if (j == i)
          {
            outputDirection[j][i] = 1.0;
          }
          else
          {
            outputDirection[j][i] = 0.0;
          }
        }
      }

      // set the spacing and origin
      outputPtr->SetSpacing(outputSpacing);
      outputPtr->SetOrigin(outputOrigin);
      outputPtr->SetDirection(outputDirection);
      outputPtr->SetNumberOfComponentsPerPixel(  // propagate vector length info
        inputPtr->GetNumberOfComponentsPerPixel());
    }
    else
    {
      // pointer could not be cast back down
      itkExceptionMacro(<< "itk::IndexedUnaryFunctorImageFilter::GenerateOutputInformation "
        << "cannot cast input to "
        << typeid(ImageBase< Superclass::InputImageDimension > *).name());
    }
  }


  template< typename TInputImage, typename TOutputImage, typename TFunction  >
  void
    IndexedUnaryFunctorImageFilter< TInputImage, TOutputImage, TFunction >
    ::ThreadedGenerateData(const OutputImageRegionType & outputRegionForThread,
    ThreadIdType threadId)
  {
    const typename OutputImageRegionType::SizeType &regionSize = outputRegionForThread.GetSize();

    if (regionSize[0] == 0)
    {
      return;
    }
    const TInputImage *inputPtr = this->GetInput();
    TOutputImage *outputPtr = this->GetOutput(0);

    // Define the portion of the input to walk for this thread, using
    // the CallCopyOutputRegionToInputRegion method allows for the input
    // and output images to be different dimensions
    InputImageRegionType inputRegionForThread;

    this->CallCopyOutputRegionToInputRegion(inputRegionForThread, outputRegionForThread);

    const size_t numberOfLinesToProcess = outputRegionForThread.GetNumberOfPixels() / regionSize[0];
    ProgressReporter progress(this, threadId, numberOfLinesToProcess);

    // Define the iterators
    ImageRegionConstIteratorWithIndex< TInputImage > inputIt(inputPtr, inputRegionForThread);
    ImageRegionIterator< TOutputImage > outputIt(outputPtr, outputRegionForThread);

    inputIt.GoToBegin();
    outputIt.GoToBegin();
    while (!inputIt.IsAtEnd())
    {
      outputIt.Set(m_Functor(inputIt.Get(), inputIt.GetIndex()));
      ++inputIt;
      ++outputIt;
      progress.CompletedPixel();  // potential exception thrown here
    }
  }
} // end namespace itk

#endif
