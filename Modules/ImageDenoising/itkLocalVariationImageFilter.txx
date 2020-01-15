/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

/*===================================================================

This file is based heavily on a corresponding ITK filter.

===================================================================*/
#ifndef _itkLocalVariationImageFilter_txx
#define _itkLocalVariationImageFilter_txx
#include "itkLocalVariationImageFilter.h"

#include "itkConstShapedNeighborhoodIterator.h"
#include "itkImageRegionIterator.h"
#include "itkNeighborhoodAlgorithm.h"
#include "itkNeighborhoodInnerProduct.h"
#include "itkOffset.h"
#include "itkProgressReporter.h"
#include "itkVectorImage.h"
#include "itkZeroFluxNeumannBoundaryCondition.h"

#include <algorithm>
#include <vector>

namespace itk
{
  template <class TInputImage, class TOutputImage>
  LocalVariationImageFilter<TInputImage, TOutputImage>::LocalVariationImageFilter()
  {
  }

  template <class TInputImage, class TOutputImage>
  void LocalVariationImageFilter<TInputImage, TOutputImage>::GenerateInputRequestedRegion()
  {
    // call the superclass' implementation of this method
    Superclass::GenerateInputRequestedRegion();

    // get pointers to the input and output
    typename Superclass::InputImagePointer inputPtr = const_cast<TInputImage *>(this->GetInput());
    typename Superclass::OutputImagePointer outputPtr = this->GetOutput();

    if (!inputPtr || !outputPtr)
    {
      return;
    }

    // get a copy of the input requested region (should equal the output
    // requested region)
    typename TInputImage::RegionType inputRequestedRegion;
    inputRequestedRegion = inputPtr->GetRequestedRegion();

    // pad the input requested region by 1
    inputRequestedRegion.PadByRadius(1);

    // crop the input requested region at the input's largest possible region
    if (inputRequestedRegion.Crop(inputPtr->GetLargestPossibleRegion()))
    {
      inputPtr->SetRequestedRegion(inputRequestedRegion);
      return;
    }
    else
    {
      // Couldn't crop the region (requested region is outside the largest
      // possible region).  Throw an exception.

      // store what we tried to request (prior to trying to crop)
      inputPtr->SetRequestedRegion(inputRequestedRegion);

      // build an exception
      InvalidRequestedRegionError e(__FILE__, __LINE__);
      e.SetLocation(ITK_LOCATION);
      e.SetDescription("Requested region outside possible region.");
      e.SetDataObject(inputPtr);
      throw e;
    }
  }

  template <>
  double SquaredEuclideanMetric<itk::VariableLengthVector<float>>::Calc(itk::VariableLengthVector<float> p)
  {
    return p.GetSquaredNorm();
  }

  template <>
  double SquaredEuclideanMetric<itk::VariableLengthVector<double>>::Calc(itk::VariableLengthVector<double> p)
  {
    return p.GetSquaredNorm();
  }

  template <class TPixelType>
  double SquaredEuclideanMetric<TPixelType>::Calc(TPixelType p)
  {
    return p * p;
  }

  template <class TInputImage, class TOutputImage>
  void LocalVariationImageFilter<TInputImage, TOutputImage>::ThreadedGenerateData(
    const OutputImageRegionType &outputRegionForThread, ThreadIdType threadId)
  {
    // Allocate output
    typename OutputImageType::Pointer output = this->GetOutput();
    typename InputImageType::ConstPointer input = this->GetInput();

    itk::Size<InputImageDimension> size;
    for (unsigned int i = 0; i < InputImageDimension; i++)
      size[i] = 1;

    // Find the data-set boundary "faces"
    NeighborhoodAlgorithm::ImageBoundaryFacesCalculator<InputImageType> bC;
    typename NeighborhoodAlgorithm::ImageBoundaryFacesCalculator<InputImageType>::FaceListType faceList =
      bC(input, outputRegionForThread, size);

    // support progress methods/callbacks
    ProgressReporter progress(this, threadId, outputRegionForThread.GetNumberOfPixels());

    ZeroFluxNeumannBoundaryCondition<InputImageType> nbc;
    std::vector<InputPixelType> pixels;

    // Process each of the boundary faces.  These are N-d regions which border
    // the edge of the buffer.
    for (auto fit = faceList.begin(); fit != faceList.end(); ++fit)
    {
      // iterators over output and input
      ImageRegionIterator<OutputImageType> output_image_it(output, *fit);
      ImageRegionConstIterator<InputImageType> input_image_it(input.GetPointer(), *fit);

      // neighborhood iterator for input image
      ConstShapedNeighborhoodIterator<InputImageType> input_image_neighbors_it(size, input, *fit);
      typename ConstShapedNeighborhoodIterator<InputImageType>::OffsetType offset;
      input_image_neighbors_it.OverrideBoundaryCondition(&nbc);
      input_image_neighbors_it.ClearActiveList();
      for (unsigned int i = 0; i < InputImageDimension; i++)
      {
        offset.Fill(0);
        offset[i] = -1;
        input_image_neighbors_it.ActivateOffset(offset);
        offset[i] = 1;
        input_image_neighbors_it.ActivateOffset(offset);
      }
      input_image_neighbors_it.GoToBegin();
      // const unsigned int neighborhoodSize = InputImageDimension*2;

      while (!input_image_neighbors_it.IsAtEnd())
      {
        // collect all the pixels in the neighborhood, note that we use
        // GetPixel on the NeighborhoodIterator to honor the boundary conditions
        typename OutputImageType::PixelType locVariation = 0;
        typename ConstShapedNeighborhoodIterator<InputImageType>::ConstIterator input_neighbors_it;
        for (input_neighbors_it = input_image_neighbors_it.Begin(); !input_neighbors_it.IsAtEnd(); input_neighbors_it++)
        {
          typename TInputImage::PixelType diffVec = input_neighbors_it.Get() - input_image_it.Get();
          locVariation += SquaredEuclideanMetric<typename TInputImage::PixelType>::Calc(diffVec);
        }
        locVariation = sqrt(locVariation + 0.0001);
        output_image_it.Set(locVariation);

        // update iterators
        ++input_image_neighbors_it;
        ++output_image_it;
        ++input_image_it;

        // report progress
        progress.CompletedPixel();
      }
    }
  }

  /**
  * Standard "PrintSelf" method
  */
  template <class TInputImage, class TOutput>
  void LocalVariationImageFilter<TInputImage, TOutput>::PrintSelf(std::ostream &os, Indent indent) const
  {
    Superclass::PrintSelf(os, indent);
  }

} // end namespace itk

#endif //_itkLocalVariationImageFilter_txx
