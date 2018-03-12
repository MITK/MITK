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
#ifndef __itkEnhancedScalarImageToRunLengthMatrixFilter_hxx
#define __itkEnhancedScalarImageToRunLengthMatrixFilter_hxx

#include "itkEnhancedScalarImageToRunLengthMatrixFilter.h"

#include "itkConstNeighborhoodIterator.h"
#include "itkNeighborhood.h"
#include "vnl/vnl_math.h"
#include "itkMacro.h"

namespace itk
{
  namespace Statistics
  {
    template<typename TImageType, typename THistogramFrequencyContainer>
    EnhancedScalarImageToRunLengthMatrixFilter<TImageType, THistogramFrequencyContainer>
      ::EnhancedScalarImageToRunLengthMatrixFilter() :
    m_NumberOfBinsPerAxis( itkGetStaticConstMacro( DefaultBinsPerAxis ) ),
      m_Min( NumericTraits<PixelType>::NonpositiveMin() ),
      m_Max( NumericTraits<PixelType>::max() ),
      m_MinDistance( NumericTraits<RealType>::ZeroValue() ),
      m_MaxDistance( NumericTraits<RealType>::max() ),
      m_InsidePixelValue( NumericTraits<PixelType>::OneValue() )
    {
      this->SetNumberOfRequiredInputs( 1 );
      this->SetNumberOfRequiredOutputs( 1 );

      const unsigned int measurementVectorSize = 2;

      this->ProcessObject::SetNthOutput( 0, this->MakeOutput( 0 ) );
      HistogramType *output = const_cast<HistogramType *>( this->GetOutput() );
      output->SetMeasurementVectorSize( measurementVectorSize );

      this->m_LowerBound.SetSize( measurementVectorSize );
      this->m_UpperBound.SetSize( measurementVectorSize );

      this->m_LowerBound[0] = this->m_Min;
      this->m_LowerBound[1] = this->m_MinDistance;
      this->m_UpperBound[0] = this->m_Max;
      this->m_UpperBound[1] = this->m_MaxDistance;
    }

    template<typename TImageType, typename THistogramFrequencyContainer>
    void
      EnhancedScalarImageToRunLengthMatrixFilter<TImageType, THistogramFrequencyContainer>
      ::SetOffset( const OffsetType offset )
    {
      OffsetVectorPointer offsetVector = OffsetVector::New();
      offsetVector->push_back( offset );
      this->SetOffsets( offsetVector );
    }

    template<typename TImageType, typename THistogramFrequencyContainer>
    void
      EnhancedScalarImageToRunLengthMatrixFilter<TImageType, THistogramFrequencyContainer>
      ::SetInput( const ImageType *image )
    {
      // Process object is not const-correct so the const_cast is required here
      this->ProcessObject::SetNthInput( 0, const_cast<ImageType *>( image ) );
    }

    template<typename TImageType, typename THistogramFrequencyContainer>
    void
      EnhancedScalarImageToRunLengthMatrixFilter<TImageType, THistogramFrequencyContainer>
      ::SetMaskImage( const ImageType *image )
    {
      // Process object is not const-correct so the const_cast is required here
      this->ProcessObject::SetNthInput( 1, const_cast<ImageType *>( image ) );
    }

    template<typename TImageType, typename THistogramFrequencyContainer>
    const TImageType *
      EnhancedScalarImageToRunLengthMatrixFilter<TImageType, THistogramFrequencyContainer>
      ::GetInput() const
    {
      if( this->GetNumberOfInputs() < 1 )
      {
        return ITK_NULLPTR;
      }
      return static_cast<const ImageType *>( this->ProcessObject::GetInput( 0 ) );
    }

    template<typename TImageType, typename THistogramFrequencyContainer>
    const TImageType *
      EnhancedScalarImageToRunLengthMatrixFilter<TImageType, THistogramFrequencyContainer>
      ::GetMaskImage() const
    {
      if( this->GetNumberOfInputs() < 2 )
      {
        return ITK_NULLPTR;
      }
      return static_cast<const ImageType *>( this->ProcessObject::GetInput( 1 ) );
    }

    template<typename TImageType, typename THistogramFrequencyContainer>
    const typename EnhancedScalarImageToRunLengthMatrixFilter<TImageType,
      THistogramFrequencyContainer >::HistogramType *
      EnhancedScalarImageToRunLengthMatrixFilter<TImageType, THistogramFrequencyContainer>
      ::GetOutput() const
    {
      const HistogramType *output =
        static_cast<const HistogramType *>( this->ProcessObject::GetOutput( 0 ) );
      return output;
    }

    template<typename TImageType, typename THistogramFrequencyContainer>
    typename EnhancedScalarImageToRunLengthMatrixFilter<TImageType,
      THistogramFrequencyContainer>::DataObjectPointer
      EnhancedScalarImageToRunLengthMatrixFilter<TImageType, THistogramFrequencyContainer>
      ::MakeOutput( DataObjectPointerArraySizeType itkNotUsed( idx ) )
    {
      return HistogramType::New().GetPointer();
    }

    template<typename TImageType, typename THistogramFrequencyContainer>
    void
      EnhancedScalarImageToRunLengthMatrixFilter<TImageType, THistogramFrequencyContainer>
      ::GenerateData()
    {
      HistogramType *output =
        static_cast<HistogramType *>( this->ProcessObject::GetOutput( 0 ) );

      const ImageType * inputImage = this->GetInput();

      // First, create an appropriate histogram with the right number of bins
      // and mins and maxes correct for the image type.
      typename HistogramType::SizeType size( output->GetMeasurementVectorSize() );

      size.Fill( this->m_NumberOfBinsPerAxis );
      this->m_LowerBound[0] = this->m_Min;
      this->m_LowerBound[1] = this->m_MinDistance;
      this->m_UpperBound[0] = this->m_Max;
      this->m_UpperBound[1] = this->m_MaxDistance;
      output->Initialize( size, this->m_LowerBound, this->m_UpperBound );

      MeasurementVectorType run( output->GetMeasurementVectorSize() );
      typename HistogramType::IndexType hIndex;

      // Iterate over all of those pixels and offsets, adding each
      // distance/intensity pair to the histogram

      typedef ConstNeighborhoodIterator<ImageType> NeighborhoodIteratorType;
      typename NeighborhoodIteratorType::RadiusType radius;
      radius.Fill( 1 );
      NeighborhoodIteratorType neighborIt( radius,
        inputImage, inputImage->GetRequestedRegion() );

      // this temp image has the same dimension for each offset
      // moving the allocation out of loop of offsets
      // while keeping FillBuffer with boolean false in each loop
      typedef Image<bool, ImageDimension> BoolImageType;
      typename BoolImageType::Pointer alreadyVisitedImage = BoolImageType::New();
      alreadyVisitedImage->CopyInformation( inputImage );
      alreadyVisitedImage->SetRegions( inputImage->GetRequestedRegion() );
      alreadyVisitedImage->Allocate();

      typename OffsetVector::ConstIterator offsets;
      for( offsets = this->GetOffsets()->Begin();
        offsets != this->GetOffsets()->End(); offsets++ )
      {
        alreadyVisitedImage->FillBuffer( false );

        neighborIt.GoToBegin();
        OffsetType offset = offsets.Value();

        this->NormalizeOffsetDirection(offset);

        for( neighborIt.GoToBegin(); !neighborIt.IsAtEnd(); ++neighborIt )
        {
          const PixelType centerPixelIntensity = neighborIt.GetCenterPixel();
          if (centerPixelIntensity != centerPixelIntensity) // Check for invalid values
          {
            continue;
          }
          IndexType centerIndex = neighborIt.GetIndex();
          if( centerPixelIntensity < this->m_Min ||
            centerPixelIntensity > this->m_Max ||
            alreadyVisitedImage->GetPixel( centerIndex ) || ( this->GetMaskImage() &&
            this->GetMaskImage()->GetPixel( centerIndex ) !=
            this->m_InsidePixelValue ) )
          {
            continue; // don't put a pixel in the histogram if the value
            // is out-of-bounds or is outside the mask.
          }

          itkDebugMacro("===> offset = " << offset << std::endl);

          MeasurementType centerBinMin = this->GetOutput()->
            GetBinMinFromValue( 0, centerPixelIntensity );
          MeasurementType centerBinMax = this->GetOutput()->
            GetBinMaxFromValue( 0, centerPixelIntensity );
          MeasurementType lastBinMax = this->GetOutput()->
            GetDimensionMaxs( 0 )[ this->GetOutput()->GetSize( 0 ) - 1 ];

          PixelType pixelIntensity( NumericTraits<PixelType>::ZeroValue() );
          IndexType index;

          int steps = 0;
          index = centerIndex + offset;
          IndexType lastGoodIndex = centerIndex;
          bool runLengthSegmentAlreadyVisited = false;

          // Scan from the current pixel at index, following
          // the direction of offset. Run length is computed as the
          // length of continuous pixels whose pixel values are
          // in the same bin.

          while ( inputImage->GetRequestedRegion().IsInside(index) )
          {
            pixelIntensity = inputImage->GetPixel(index);
            // For the same offset, each run length segment can
            // only be visited once
            if (alreadyVisitedImage->GetPixel( index ) )
            {
              runLengthSegmentAlreadyVisited = true;
              break;
            }
            if (pixelIntensity != pixelIntensity)
            {
              break;
            }


            // Special attention paid to boundaries of bins.
            // For the last bin,
            // it is left close and right close (following the previous
            // gerrit patch).
            // For all
            // other bins,
            // the bin is left close and right open.

            if ( pixelIntensity >= centerBinMin
              && ( pixelIntensity < centerBinMax || ( pixelIntensity == centerBinMax && centerBinMax == lastBinMax ) )
              && (!this->GetMaskImage() || this->GetMaskImage()->GetPixel(index) == this->m_InsidePixelValue))
            {
              alreadyVisitedImage->SetPixel( index, true );
              lastGoodIndex = index;
              index += offset;
              steps++;
            }
            else
            {
              break;
            }
          }

          if ( runLengthSegmentAlreadyVisited )
          {
            MITK_INFO << "Already visited 1 " << index;
            continue;
          }
          IndexType lastGoodIndex2 = lastGoodIndex;
          index = centerIndex - offset;
          lastGoodIndex = centerIndex;
          while ( inputImage->GetRequestedRegion().IsInside(index) )
          {
            pixelIntensity = inputImage->GetPixel(index);
            if (pixelIntensity != pixelIntensity)
            {
              break;
            }
            if (alreadyVisitedImage->GetPixel( index ) )
            {
              if (pixelIntensity >= centerBinMin
                && (pixelIntensity < centerBinMax || (pixelIntensity == centerBinMax && centerBinMax == lastBinMax)))
              {
                runLengthSegmentAlreadyVisited = true;
              }
              break;
            }

            if ( pixelIntensity >= centerBinMin
              && ( pixelIntensity < centerBinMax || ( pixelIntensity == centerBinMax && centerBinMax == lastBinMax ) )
              && (!this->GetMaskImage() || this->GetMaskImage()->GetPixel(index) == this->m_InsidePixelValue))
            {
              alreadyVisitedImage->SetPixel( index, true );
              lastGoodIndex = index;
              steps++;
              index -= offset;
            }
            else
              break;
          }
          if (runLengthSegmentAlreadyVisited)
          {
            MITK_INFO << "Already visited 2 " << index;
            continue;
          }
          PointType centerPoint;
          inputImage->TransformIndexToPhysicalPoint(
            centerIndex, centerPoint );
          PointType point;
          inputImage->TransformIndexToPhysicalPoint( lastGoodIndex, point );
          PointType point2;
          inputImage->TransformIndexToPhysicalPoint( lastGoodIndex2, point2 );

          run[0] = centerPixelIntensity;
          run[1] = steps;
          //run[1] = point.EuclideanDistanceTo( point2 );

          if( run[1] >= this->m_MinDistance && run[1] <= this->m_MaxDistance )
          {
            output->GetIndex( run, hIndex );
            output->IncreaseFrequencyOfIndex( hIndex, 1 );

            itkDebugStatement(typename HistogramType::IndexType tempMeasurementIndex;)
              itkDebugStatement(output->GetIndex(run,tempMeasurementIndex);)
              itkDebugMacro( "centerIndex<->index: "
              << static_cast<int>( centerPixelIntensity )
              << "@"<< centerIndex
              << "<->" << static_cast<int>( pixelIntensity ) << "@" << index
              <<", Bin# " << tempMeasurementIndex
              << ", Measurement: (" << run[0] << ", " << run[1] << ")"
              << ", Center bin [" << this->GetOutput()->GetBinMinFromValue( 0, run[0] )
              << "," << this->GetOutput()->GetBinMaxFromValue( 0, run[0] ) << "]"
              << "~[" << this->GetOutput()->GetBinMinFromValue( 1, run[1] )
              << "," << this->GetOutput()->GetBinMaxFromValue( 1, run[1] ) << "]"
              << std::endl );
          }
        }
      }
    }

    template<typename TImageType, typename THistogramFrequencyContainer>
    void
      EnhancedScalarImageToRunLengthMatrixFilter<TImageType, THistogramFrequencyContainer>
      ::SetPixelValueMinMax( PixelType min, PixelType max )
    {
      if( this->m_Min != min || this->m_Max != max )
      {
        itkDebugMacro( "setting Min to " << min << "and Max to " << max );
        this->m_Min = min;
        this->m_Max = max;
        this->Modified();
      }
    }

    template<typename TImageType, typename THistogramFrequencyContainer>
    void
      EnhancedScalarImageToRunLengthMatrixFilter<TImageType, THistogramFrequencyContainer>
      ::SetDistanceValueMinMax( RealType min, RealType max )
    {
      if( this->m_MinDistance != min || this->m_MaxDistance != max )
      {
        itkDebugMacro( "setting MinDistance to " << min << "and MaxDistance to "
          << max );
        this->m_MinDistance = min;
        this->m_MaxDistance = max;
        this->Modified();
      }
    }

    template<typename TImageType, typename THistogramFrequencyContainer>
    void
      EnhancedScalarImageToRunLengthMatrixFilter<TImageType, THistogramFrequencyContainer>
      ::PrintSelf( std::ostream& os, Indent indent ) const
    {
      Superclass::PrintSelf( os,indent );
      os << indent << "Offsets: " << this->GetOffsets() << std::endl;
      os << indent << "Min: " << this->m_Min << std::endl;
      os << indent << "Max: " << this->m_Max << std::endl;
      os << indent << "Min distance: " << this->m_MinDistance << std::endl;
      os << indent << "Max distance: " << this->m_MaxDistance << std::endl;
      os << indent << "NumberOfBinsPerAxis: " << this->m_NumberOfBinsPerAxis
        << std::endl;
      os << indent << "InsidePixelValue: " << this->m_InsidePixelValue << std::endl;
    }

    template<typename TImageType, typename THistogramFrequencyContainer>
    void
      EnhancedScalarImageToRunLengthMatrixFilter<TImageType, THistogramFrequencyContainer>
      ::NormalizeOffsetDirection(OffsetType &offset)
    {
      itkDebugMacro("old offset = " << offset << std::endl);
      int sign = 1;
      bool metLastNonZero = false;
      for (int i = offset.GetOffsetDimension()-1; i>=0; i--)
      {
        if (metLastNonZero)
        {
          offset[i] *= sign;
        }
        else if (offset[i] != 0)
        {
          sign = (offset[i] > 0 ) ? 1 : -1;
          metLastNonZero = true;
          offset[i] *= sign;
        }
      }

      itkDebugMacro("new  offset = " << offset << std::endl);
    }
  } // end of namespace Statistics
} // end of namespace itk

#endif