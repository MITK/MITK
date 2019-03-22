/*=========================================================================
 *
 *  Copyright Insight Software Consortium
 *  Copyright Samara State Medical University
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
#ifndef itkLabelMedianImageFilter_hxx
#define itkLabelMedianImageFilter_hxx
#include "itkLabelMedianImageFilter.h"

#include "itkConstNeighborhoodIterator.h"
#include "itkNeighborhoodInnerProduct.h"
#include "itkImageRegionIterator.h"
#include "itkNeighborhoodAlgorithm.h"
#include "itkOffset.h"
#include "itkProgressReporter.h"

#include <vector>
#include <algorithm>
#include "itkMath.h"

namespace itk
{
template< typename TInputImage, typename TOutputImage >
LabelMedianImageFilter< TInputImage, TOutputImage >
::LabelMedianImageFilter() : m_CenterWeight(17)
{}

template< typename TInputImage, typename TOutputImage >
void
LabelMedianImageFilter< TInputImage, TOutputImage >
::ThreadedGenerateData(const OutputImageRegionType & outputRegionForThread,
                       ThreadIdType threadId)
{
  ZeroFluxNeumannBoundaryCondition< InputImageType > nbc;

  ConstNeighborhoodIterator< InputImageType > bit;
  ImageRegionIterator< OutputImageType >      it;

  // Allocate output
  typename OutputImageType::Pointer output = this->GetOutput();
  typename InputImageType::ConstPointer input  = this->GetInput();

  // Find the data-set boundary "faces"
  typename NeighborhoodAlgorithm::ImageBoundaryFacesCalculator< InputImageType >::FaceListType faceList;
  NeighborhoodAlgorithm::ImageBoundaryFacesCalculator< InputImageType > bC;
  faceList = bC(input, outputRegionForThread, Superclass::GetRadius());

  typename NeighborhoodAlgorithm::ImageBoundaryFacesCalculator< InputImageType >::FaceListType::iterator fit;

  ProgressReporter progress( this, threadId, outputRegionForThread.GetNumberOfPixels() );

  // Process each of the boundary faces.  These are N-d regions which border
  // the edge of the buffer.
  for ( fit = faceList.begin(); fit != faceList.end(); ++fit )
    {
    bit = ConstNeighborhoodIterator< InputImageType >(Superclass::GetRadius(), input, *fit);
    it  = ImageRegionIterator< OutputImageType >(output, *fit);
    bit.OverrideBoundaryCondition(&nbc);
    bit.GoToBegin();

    unsigned int neighborhoodSize = bit.Size();

    // We can not compute mean values for an even number of elemets because it can be a new label.
    // Instead we count values which are more or less than the central.
    // Only the vast majority can change the center value.

    while ( !bit.IsAtEnd() )
      {
      InputPixelType c = bit.GetCenterPixel();
      if ( Superclass::GetBackgroundValue() <= c && c <= Superclass::GetForegroundValue())
        {
        // counting leesser, bigger and equal pixels
        unsigned int hi = 0, lo = 0, eq = m_CenterWeight;
        for ( unsigned int i = neighborhoodSize/2; i--; )
          {
          auto r = bit.GetOffset(i);
          unsigned int w = 0;
          for (auto j = Superclass::InputImageDimension; j--;)
            {
            w += std::abs(r[j]);
            }
          w = w>1? w>2 || m_CenterWeight<3 ? 1 : 2 : m_CenterWeight;
          InputPixelType v = bit.GetPixel(i);
          (v < c? lo : v > c? hi : eq) += w;
          v = bit.GetPixel(neighborhoodSize - 1 - i);
          (v < c? lo : v > c? hi : eq) += w;
          }
        it.Set( static_cast< OutputPixelType >( hi > eq + lo? c + 1 : lo > eq + hi? c - 1 : c ) );
        }
      else
        {
        it.Set( static_cast< OutputPixelType >( c ) );
        }
      ++bit;
      ++it;
      progress.CompletedPixel();
      }
    }
}

} // end namespace itk

#endif
