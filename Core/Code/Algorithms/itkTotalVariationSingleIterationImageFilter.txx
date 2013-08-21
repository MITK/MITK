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

/*===================================================================

This file is based heavily on a corresponding ITK filter.

===================================================================*/

#ifndef _itkTotalVariationSingleIterationImageFilter_txx
#define _itkTotalVariationSingleIterationImageFilter_txx

#include "itkTotalVariationSingleIterationImageFilter.h"

// itk includes
#include "itkConstShapedNeighborhoodIterator.h"
#include "itkNeighborhoodInnerProduct.h"
#include "itkImageRegionIterator.h"
#include "itkNeighborhoodAlgorithm.h"
#include "itkZeroFluxNeumannBoundaryCondition.h"
#include "itkOffset.h"
#include "itkProgressReporter.h"
#include "itkLocalVariationImageFilter.h"

// other includes
#include <vector>
#include <algorithm>

namespace itk
{

  /**
  * constructor
  */
  template <class TInputImage, class TOutputImage>
  TotalVariationSingleIterationImageFilter<TInputImage, TOutputImage>
    ::TotalVariationSingleIterationImageFilter()
  {
    m_Lambda = 1.0;
    m_LocalVariation = LocalVariationImageType::New();
  }

   /**
  * generate requested region
  */
 template <class TInputImage, class TOutputImage>
  void
    TotalVariationSingleIterationImageFilter<TInputImage, TOutputImage>
    ::GenerateInputRequestedRegion() throw (InvalidRequestedRegionError)
  {
    // call the superclass' implementation of this method
    Superclass::GenerateInputRequestedRegion();

    // get pointers to the input and output
    typename Superclass::InputImagePointer inputPtr =
      const_cast< TInputImage * >( this->GetInput() );
    typename Superclass::OutputImagePointer outputPtr = this->GetOutput();

    if ( !inputPtr || !outputPtr )
    {
      return;
    }

    // get a copy of the input requested region (should equal the output
    // requested region)
    typename TInputImage::RegionType inputRequestedRegion;
    inputRequestedRegion = inputPtr->GetRequestedRegion();

    // pad the input requested region by 1
    inputRequestedRegion.PadByRadius( 1 );

    // crop the input requested region at the input's largest possible region
    if ( inputRequestedRegion.Crop(inputPtr->GetLargestPossibleRegion()) )
    {
      inputPtr->SetRequestedRegion( inputRequestedRegion );
      return;
    }
    else
    {
      // Couldn't crop the region (requested region is outside the largest
      // possible region).  Throw an exception.

      // store what we tried to request (prior to trying to crop)
      inputPtr->SetRequestedRegion( inputRequestedRegion );

      // build an exception
      InvalidRequestedRegionError e(__FILE__, __LINE__);
      e.SetLocation(ITK_LOCATION);
      e.SetDescription("Requested region outside possible region.");
      e.SetDataObject(inputPtr);
      throw e;
    }
  }


   /**
  * generate output
  */
 template< class TInputImage, class TOutputImage>
  void
    TotalVariationSingleIterationImageFilter< TInputImage, TOutputImage>
    ::ThreadedGenerateData(const OutputImageRegionType& outputRegionForThread,
    ThreadIdType threadId)
  {

    typename OutputImageType::Pointer output = this->GetOutput();
    typename  InputImageType::ConstPointer input  = this->GetInput();

    // Find the data-set boundary "faces"
    itk::Size<InputImageDimension> size;
    for( int i=0; i<InputImageDimension; i++)
      size[i] = 1;

    NeighborhoodAlgorithm::ImageBoundaryFacesCalculator<InputImageType> bC;
    typename NeighborhoodAlgorithm::
      ImageBoundaryFacesCalculator<InputImageType>::FaceListType
      faceList = bC(input, outputRegionForThread, size);

    NeighborhoodAlgorithm::
      ImageBoundaryFacesCalculator<LocalVariationImageType> lv_bC;
    typename NeighborhoodAlgorithm::
      ImageBoundaryFacesCalculator<LocalVariationImageType>::FaceListType
      lv_faceList = lv_bC(m_LocalVariation, outputRegionForThread, size);

    // support progress methods/callbacks
    ProgressReporter progress(
      this, threadId, outputRegionForThread.GetNumberOfPixels());

    ZeroFluxNeumannBoundaryCondition<InputImageType> nbc;
    ZeroFluxNeumannBoundaryCondition<LocalVariationImageType> lv_nbc;
    std::vector<double> ws;
    std::vector<double> hs;

    typename NeighborhoodAlgorithm::
      ImageBoundaryFacesCalculator<InputImageType>::FaceListType::iterator
      lv_fit=lv_faceList.begin();

    // Process each of the boundary faces.  These are N-d regions which border
    // the edge of the buffer.
    for ( typename NeighborhoodAlgorithm::
      ImageBoundaryFacesCalculator<InputImageType>::FaceListType::iterator
      fit=faceList.begin(); fit != faceList.end(); ++fit)
    {

      // iterators over output, input, original and local variation image
      ImageRegionIterator<OutputImageType> output_image_it =
        ImageRegionIterator<OutputImageType>(output, *fit);
      ImageRegionConstIterator<InputImageType> input_image_it =
        ImageRegionConstIterator<InputImageType>(input, *fit);
      ImageRegionConstIterator<InputImageType> orig_image_it =
        ImageRegionConstIterator<InputImageType>(m_OriginalImage, *fit);
      ImageRegionConstIterator<LocalVariationImageType> loc_var_image_it =
        ImageRegionConstIterator<LocalVariationImageType>(
        m_LocalVariation, *fit);

      // neighborhood in input image
      ConstShapedNeighborhoodIterator<InputImageType>
        input_image_neighbors_it(size, input, *fit);
      typename ConstShapedNeighborhoodIterator<InputImageType>::
        OffsetType offset;
      input_image_neighbors_it.OverrideBoundaryCondition(&nbc);
      input_image_neighbors_it.ClearActiveList();
      for(int i=0; i<InputImageDimension; i++)
      {
        offset.Fill(0);
        offset[i] = -1;
        input_image_neighbors_it.ActivateOffset(offset);
        offset[i] = 1;
        input_image_neighbors_it.ActivateOffset(offset);
      }
      input_image_neighbors_it.GoToBegin();

      // neighborhood in local variation image
      ConstShapedNeighborhoodIterator<LocalVariationImageType>
        loc_var_image_neighbors_it(size, m_LocalVariation, *lv_fit);
      loc_var_image_neighbors_it.OverrideBoundaryCondition(&lv_nbc);
      loc_var_image_neighbors_it.ClearActiveList();
      for(int i=0; i<InputImageDimension; i++)
      {
        offset.Fill(0);
        offset[i] = -1;
        loc_var_image_neighbors_it.ActivateOffset(offset);
        offset[i] = 1;
        loc_var_image_neighbors_it.ActivateOffset(offset);
      }
      loc_var_image_neighbors_it.GoToBegin();

      const unsigned int neighborhoodSize = InputImageDimension*2;
      ws.resize(neighborhoodSize);

      while ( ! output_image_it.IsAtEnd() )
      {

        //   1 / ||nabla_alpha(u)||_a
        double locvar_alpha_inv = 1.0/loc_var_image_it.Get();

        // compute w_alphabetas
        int count = 0;
        double wsum = 0;
        typename ConstShapedNeighborhoodIterator<LocalVariationImageType>::
          ConstIterator loc_var_neighbors_it;
        for (loc_var_neighbors_it = loc_var_image_neighbors_it.Begin();
          ! loc_var_neighbors_it.IsAtEnd();
          loc_var_neighbors_it++)
        {
          // w_alphabeta(u) =
          //   1 / ||nabla_alpha(u)||_a + 1 / ||nabla_beta(u)||_a
          ws[count] =
            locvar_alpha_inv + (1.0/(double)loc_var_neighbors_it.Get());
          wsum += ws[count++];
        }

        // h_alphaalpha * u_alpha^zero
        typename OutputImageType::PixelType res =
          static_cast<typename OutputImageType::PixelType>(
          ((typename OutputImageType::PixelType)
          orig_image_it.Get()) * (m_Lambda / (m_Lambda+wsum)));

        // add the different h_alphabeta * u_beta
        count = 0;
        typename ConstShapedNeighborhoodIterator<InputImageType>::
          ConstIterator input_neighbors_it;
        for (input_neighbors_it = input_image_neighbors_it.Begin();
          ! input_neighbors_it.IsAtEnd();
          input_neighbors_it++)
        {
          res += input_neighbors_it.Get() * (ws[count++] / (m_Lambda+wsum));
        }

        // set output result
        output_image_it.Set( res );

        // increment iterators
        ++output_image_it;
        ++input_image_it;
        ++orig_image_it;
        ++loc_var_image_it;
        ++input_image_neighbors_it;
        ++loc_var_image_neighbors_it;

        // report progress
        progress.CompletedPixel();

      }

      ++lv_fit;
    }
  }

  /**
  * first calculate local variation in the image
  */
  template< class TInputImage, class TOutputImage>
  void
    TotalVariationSingleIterationImageFilter< TInputImage, TOutputImage>
    ::BeforeThreadedGenerateData()
  {
    typedef typename itk::LocalVariationImageFilter
      <TInputImage,LocalVariationImageType> FilterType;
    typename FilterType::Pointer filter = FilterType::New();
    filter->SetInput(this->GetInput(0));
    filter->SetNumberOfThreads(this->GetNumberOfThreads());
    filter->Update();
    this->m_LocalVariation = filter->GetOutput();
  }

  /**
  * Standard "PrintSelf" method
  */
  template <class TInputImage, class TOutput>
  void
    TotalVariationSingleIterationImageFilter<TInputImage, TOutput>
    ::PrintSelf(
    std::ostream& os,
    Indent indent) const
  {
    Superclass::PrintSelf( os, indent );
  }

} // end namespace itk

#endif
