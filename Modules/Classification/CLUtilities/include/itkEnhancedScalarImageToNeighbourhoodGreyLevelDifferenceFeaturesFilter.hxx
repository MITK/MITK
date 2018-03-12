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
#ifndef __itkEnhancedScalarImageToNeighbourhoodGreyLevelDifferenceFeaturesFilter_hxx
#define __itkEnhancedScalarImageToNeighbourhoodGreyLevelDifferenceFeaturesFilter_hxx

#include "itkEnhancedScalarImageToNeighbourhoodGreyLevelDifferenceFeaturesFilter.h"
#include "itkNeighborhood.h"
#include <itkImageRegionConstIterator.h>
#include "vnl/vnl_math.h"

namespace itk
{
  namespace Statistics
  {
    template<typename TImage, typename THistogramFrequencyContainer>
    EnhancedScalarImageToNeighbourhoodGreyLevelDifferenceFeaturesFilter<TImage, THistogramFrequencyContainer>
      ::EnhancedScalarImageToNeighbourhoodGreyLevelDifferenceFeaturesFilter()
    {
      this->SetNumberOfRequiredInputs( 1 );
      this->SetNumberOfRequiredOutputs( 1 );

      for( int i = 0; i < 2; ++i )
      {
        this->ProcessObject::SetNthOutput( i, this->MakeOutput( i ) );
      }

      this->m_NeighbourhoodGreyLevelDifferenceMatrixGenerator = NeighbourhoodGreyLevelDifferenceMatrixFilterType::New();
      this->m_FeatureMeans = FeatureValueVector::New();
      this->m_FeatureStandardDeviations = FeatureValueVector::New();

      // Set the requested features to the default value:
      // {Energy, Entropy, InverseDifferenceMoment, Inertia, ClusterShade,
      // ClusterProminence}
      FeatureNameVectorPointer requestedFeatures = FeatureNameVector::New();
      // can't directly set this->m_RequestedFeatures since it is const!

      requestedFeatures->push_back( NeighbourhoodGreyLevelDifferenceFeaturesFilterType::Coarseness );
      requestedFeatures->push_back( NeighbourhoodGreyLevelDifferenceFeaturesFilterType::Contrast );
      requestedFeatures->push_back( NeighbourhoodGreyLevelDifferenceFeaturesFilterType::Busyness );
      requestedFeatures->push_back( NeighbourhoodGreyLevelDifferenceFeaturesFilterType::Complexity );
      requestedFeatures->push_back( NeighbourhoodGreyLevelDifferenceFeaturesFilterType::Strength );
      requestedFeatures->push_back( 20 );

      this->SetRequestedFeatures( requestedFeatures );

      // Set the offset directions to their defaults: half of all the possible
      // directions 1 pixel away. (The other half is included by symmetry.)
      // We use a neighborhood iterator to calculate the appropriate offsets.
      typedef Neighborhood<typename ImageType::PixelType,
        ImageType::ImageDimension> NeighborhoodType;
      NeighborhoodType hood;
      hood.SetRadius( 1 );

      // select all "previous" neighbors that are face+edge+vertex
      // connected to the current pixel. do not include the center pixel.
      unsigned int centerIndex = hood.GetCenterNeighborhoodIndex();
      OffsetVectorPointer offsets = OffsetVector::New();
      for( unsigned int d = 0; d < centerIndex; d++ )
      {
        OffsetType offset = hood.GetOffset( d );
        offsets->push_back( offset );
      }
      this->SetOffsets( offsets );
      this->m_FastCalculations = false;
    }

    template<typename TImage, typename THistogramFrequencyContainer>
    typename
      EnhancedScalarImageToNeighbourhoodGreyLevelDifferenceFeaturesFilter<TImage, THistogramFrequencyContainer>
      ::DataObjectPointer
      EnhancedScalarImageToNeighbourhoodGreyLevelDifferenceFeaturesFilter<TImage, THistogramFrequencyContainer>
      ::MakeOutput( DataObjectPointerArraySizeType itkNotUsed(idx) )
    {
      return FeatureValueVectorDataObjectType::New().GetPointer();
    }

    template<typename TImage, typename THistogramFrequencyContainer>
    void
      EnhancedScalarImageToNeighbourhoodGreyLevelDifferenceFeaturesFilter<TImage, THistogramFrequencyContainer>
      ::GenerateData(void)
    {
      if ( this->m_FastCalculations )
      {
        this->FastCompute();
      }
      else
      {
        this->FullCompute();
      }
    }

    template<typename TImage, typename THistogramFrequencyContainer>
    void
      EnhancedScalarImageToNeighbourhoodGreyLevelDifferenceFeaturesFilter<TImage, THistogramFrequencyContainer>
      ::FullCompute()
    {
      int numFeatures = this->m_RequestedFeatures->size();
      double *features = new double [numFeatures];

      unsigned long numberOfVoxels = 0;
      ImageRegionConstIterator<TImage> voxelCountIter(this->GetMaskImage(),this->GetMaskImage()->GetLargestPossibleRegion());
      while ( ! voxelCountIter.IsAtEnd() )
      {
        if (voxelCountIter.Get() > 0)
          ++numberOfVoxels;
        ++voxelCountIter;
      }

      // For each offset, calculate each feature
      typename OffsetVector::ConstIterator offsetIt;
      int offsetNum, featureNum;
      typedef typename NeighbourhoodGreyLevelDifferenceFeaturesFilterType::NeighbourhoodGreyLevelDifferenceFeatureName
        InternalNeighbourhoodGreyLevelDifferenceFeatureName;

      std::vector <OffsetType> tVector;
      for( offsetIt = this->m_Offsets->Begin(), offsetNum = 0;
        offsetIt != this->m_Offsets->End(); offsetIt++, offsetNum++ )
      {
        MITK_INFO << "Adding offset " << offsetIt.Value();
        tVector.push_back(offsetIt.Value());
      }
      this->m_NeighbourhoodGreyLevelDifferenceMatrixGenerator->AddOffsets( tVector);
        this->m_NeighbourhoodGreyLevelDifferenceMatrixGenerator->Update();
        typename NeighbourhoodGreyLevelDifferenceFeaturesFilterType::Pointer NeighbourhoodGreyLevelDifferenceMatrixCalculator =
          NeighbourhoodGreyLevelDifferenceFeaturesFilterType::New();
        NeighbourhoodGreyLevelDifferenceMatrixCalculator->SetInput(
          this->m_NeighbourhoodGreyLevelDifferenceMatrixGenerator->GetOutput() );
        NeighbourhoodGreyLevelDifferenceMatrixCalculator->SetSiMatrix(
          this->m_NeighbourhoodGreyLevelDifferenceMatrixGenerator->GetSiMatrix() );
        NeighbourhoodGreyLevelDifferenceMatrixCalculator->SetNumberOfVoxels(numberOfVoxels);
        NeighbourhoodGreyLevelDifferenceMatrixCalculator->Update();

        typename FeatureNameVector::ConstIterator fnameIt;
        for( fnameIt = this->m_RequestedFeatures->Begin(), featureNum = 0;
          fnameIt != this->m_RequestedFeatures->End(); fnameIt++, featureNum++ )
        {
          InternalNeighbourhoodGreyLevelDifferenceFeatureName tn = ( InternalNeighbourhoodGreyLevelDifferenceFeatureName )fnameIt.Value();
          double xx = NeighbourhoodGreyLevelDifferenceMatrixCalculator->GetFeature(tn);

          features[featureNum] = xx;
        }


      // Now get the mean and deviaton of each feature across the offsets.
      this->m_FeatureMeans->clear();
      this->m_FeatureStandardDeviations->clear();
      double *tempFeatureMeans = new double[numFeatures];
      double *tempFeatureDevs = new double[numFeatures];

      /*Compute incremental mean and SD, a la Knuth, "The  Art of Computer
      Programming, Volume 2: Seminumerical Algorithms",  section 4.2.2.
      Compute mean and standard deviation using the recurrence relation:
      M(1) = x(1), M(k) = M(k-1) + (x(k) - M(k-1) ) / k
      S(1) = 0, S(k) = S(k-1) + (x(k) - M(k-1)) * (x(k) - M(k))
      for 2 <= k <= n, then
      sigma = std::sqrt(S(n) / n) (or divide by n-1 for sample SD instead of
      population SD).
      */

      // Set up the initial conditions (k = 1)
      for( featureNum = 0; featureNum < numFeatures; featureNum++ )
      {
        this->m_FeatureMeans->push_back( features[featureNum] );
        //tempFeatureMeans[featureNum] = features[0][featureNum];
        //tempFeatureDevs[featureNum] = 0;
      }
      // Zone through the recurrence (k = 2 ... N)
      /*for( offsetNum = 1; offsetNum < numOffsets; offsetNum++ )
      {
        int k = offsetNum + 1;
        for( featureNum = 0; featureNum < numFeatures; featureNum++ )
        {
          double M_k_minus_1 = tempFeatureMeans[featureNum];
          double S_k_minus_1 = tempFeatureDevs[featureNum];
          double x_k = features[offsetNum][featureNum];

          double M_k = M_k_minus_1 + ( x_k - M_k_minus_1 ) / k;
          double S_k = S_k_minus_1 + ( x_k - M_k_minus_1 ) * ( x_k - M_k );

          tempFeatureMeans[featureNum] = M_k;
          tempFeatureDevs[featureNum] = S_k;
        }
      }
      for( featureNum = 0; featureNum < numFeatures; featureNum++ )
      {
        tempFeatureDevs[featureNum] = std::sqrt( tempFeatureDevs[featureNum] /
          numOffsets );

        this->m_FeatureMeans->push_back( tempFeatureMeans[featureNum] );
        this->m_FeatureStandardDeviations->push_back( tempFeatureDevs[featureNum] );
      }
      */

      FeatureValueVectorDataObjectType *meanOutputObject =
        itkDynamicCastInDebugMode<FeatureValueVectorDataObjectType *>( this->ProcessObject::GetOutput( 0 ) );
      meanOutputObject->Set( this->m_FeatureMeans );

      FeatureValueVectorDataObjectType *standardDeviationOutputObject =
        itkDynamicCastInDebugMode<FeatureValueVectorDataObjectType *>( this->ProcessObject::GetOutput( 1 ) );
      standardDeviationOutputObject->Set( this->m_FeatureStandardDeviations );

      delete[] tempFeatureMeans;
      delete[] tempFeatureDevs;
      /*for( int i = 0; i < numOffsets; i++ )
      {
        delete[] features[i];
      }*/
      delete[] features;
    }

    template<typename TImage, typename THistogramFrequencyContainer>
    void
      EnhancedScalarImageToNeighbourhoodGreyLevelDifferenceFeaturesFilter<TImage, THistogramFrequencyContainer>
      ::FastCompute()
    {
      // Compute the feature for the first offset
      typename OffsetVector::ConstIterator offsetIt = this->m_Offsets->Begin();
      this->m_NeighbourhoodGreyLevelDifferenceMatrixGenerator->SetOffset( offsetIt.Value() );

      this->m_NeighbourhoodGreyLevelDifferenceMatrixGenerator->Update();
      typename NeighbourhoodGreyLevelDifferenceFeaturesFilterType::Pointer NeighbourhoodGreyLevelDifferenceMatrixCalculator =
        NeighbourhoodGreyLevelDifferenceFeaturesFilterType::New();
      NeighbourhoodGreyLevelDifferenceMatrixCalculator->SetInput(
        this->m_NeighbourhoodGreyLevelDifferenceMatrixGenerator->GetOutput() );
      NeighbourhoodGreyLevelDifferenceMatrixCalculator->SetSiMatrix(
        this->m_NeighbourhoodGreyLevelDifferenceMatrixGenerator->GetSiMatrix() );
      NeighbourhoodGreyLevelDifferenceMatrixCalculator->Update();

      typedef typename NeighbourhoodGreyLevelDifferenceFeaturesFilterType::NeighbourhoodGreyLevelDifferenceFeatureName
        InternalNeighbourhoodGreyLevelDifferenceFeatureName;
      this->m_FeatureMeans->clear();
      this->m_FeatureStandardDeviations->clear();
      typename FeatureNameVector::ConstIterator fnameIt;
      for( fnameIt = this->m_RequestedFeatures->Begin();
        fnameIt != this->m_RequestedFeatures->End(); fnameIt++ )
      {
        this->m_FeatureMeans->push_back( NeighbourhoodGreyLevelDifferenceMatrixCalculator->GetFeature(
          ( InternalNeighbourhoodGreyLevelDifferenceFeatureName )fnameIt.Value() ) );
        this->m_FeatureStandardDeviations->push_back( 0.0 );
      }

      FeatureValueVectorDataObjectType *meanOutputObject =
        itkDynamicCastInDebugMode<FeatureValueVectorDataObjectType *>( this->ProcessObject::GetOutput( 0 ) );
      meanOutputObject->Set( this->m_FeatureMeans );

      FeatureValueVectorDataObjectType *standardDeviationOutputObject =
        itkDynamicCastInDebugMode<FeatureValueVectorDataObjectType *>( this->ProcessObject::GetOutput( 1 ) );
      standardDeviationOutputObject->Set( this->m_FeatureStandardDeviations );
    }

    template<typename TImage, typename THistogramFrequencyContainer>
    void
      EnhancedScalarImageToNeighbourhoodGreyLevelDifferenceFeaturesFilter<TImage, THistogramFrequencyContainer>
      ::SetInput( const ImageType *image )
    {
      // Process object is not const-correct so the const_cast is required here
      this->ProcessObject::SetNthInput( 0,
        const_cast<ImageType *>( image ) );

      this->m_NeighbourhoodGreyLevelDifferenceMatrixGenerator->SetInput( image );
    }

    template<typename TImage, typename THistogramFrequencyContainer>
    void
      EnhancedScalarImageToNeighbourhoodGreyLevelDifferenceFeaturesFilter<TImage, THistogramFrequencyContainer>
      ::SetNumberOfBinsPerAxis( unsigned int numberOfBins )
    {
      itkDebugMacro( "setting NumberOfBinsPerAxis to " << numberOfBins );
      this->m_NeighbourhoodGreyLevelDifferenceMatrixGenerator->SetNumberOfBinsPerAxis( numberOfBins );
      this->Modified();
    }

    template<typename TImage, typename THistogramFrequencyContainer>
    void
      EnhancedScalarImageToNeighbourhoodGreyLevelDifferenceFeaturesFilter<TImage, THistogramFrequencyContainer>
      ::SetPixelValueMinMax( PixelType min, PixelType max )
    {
      itkDebugMacro( "setting Min to " << min << "and Max to " << max );
      this->m_NeighbourhoodGreyLevelDifferenceMatrixGenerator->SetPixelValueMinMax( min, max );
      this->Modified();
    }

    template<typename TImage, typename THistogramFrequencyContainer>
    void
      EnhancedScalarImageToNeighbourhoodGreyLevelDifferenceFeaturesFilter<TImage, THistogramFrequencyContainer>
      ::SetDistanceValueMinMax( double min, double max )
    {
      itkDebugMacro( "setting Min to " << min << "and Max to " << max );
      this->m_NeighbourhoodGreyLevelDifferenceMatrixGenerator->SetDistanceValueMinMax( min, max );
      this->Modified();
    }

    template<typename TImage, typename THistogramFrequencyContainer>
    void
      EnhancedScalarImageToNeighbourhoodGreyLevelDifferenceFeaturesFilter<TImage, THistogramFrequencyContainer>
      ::SetMaskImage( const ImageType *image )
    {
      // Process object is not const-correct so the const_cast is required here
      this->ProcessObject::SetNthInput( 1,
        const_cast< ImageType * >( image ) );

      this->m_NeighbourhoodGreyLevelDifferenceMatrixGenerator->SetMaskImage( image );
    }

    template<typename TImage, typename THistogramFrequencyContainer>
    const TImage *
      EnhancedScalarImageToNeighbourhoodGreyLevelDifferenceFeaturesFilter<TImage, THistogramFrequencyContainer>
      ::GetInput() const
    {
      if ( this->GetNumberOfInputs() < 1 )
      {
        return ITK_NULLPTR;
      }
      return static_cast<const ImageType *>( this->ProcessObject::GetInput( 0 ) );
    }

    template<typename TImage, typename THistogramFrequencyContainer>
    const typename
      EnhancedScalarImageToNeighbourhoodGreyLevelDifferenceFeaturesFilter<TImage, THistogramFrequencyContainer>
      ::FeatureValueVectorDataObjectType *
      EnhancedScalarImageToNeighbourhoodGreyLevelDifferenceFeaturesFilter<TImage, THistogramFrequencyContainer>
      ::GetFeatureMeansOutput() const
    {
      return itkDynamicCastInDebugMode<const FeatureValueVectorDataObjectType *>
        (this->ProcessObject::GetOutput( 0 ) );
    }

    template<typename TImage, typename THistogramFrequencyContainer>
    const typename
      EnhancedScalarImageToNeighbourhoodGreyLevelDifferenceFeaturesFilter<TImage, THistogramFrequencyContainer>
      ::FeatureValueVectorDataObjectType *
      EnhancedScalarImageToNeighbourhoodGreyLevelDifferenceFeaturesFilter<TImage, THistogramFrequencyContainer>
      ::GetFeatureStandardDeviationsOutput() const
    {
      return itkDynamicCastInDebugMode< const FeatureValueVectorDataObjectType * >
        ( this->ProcessObject::GetOutput( 1 ) );
    }

    template<typename TImage, typename THistogramFrequencyContainer>
    const TImage *
      EnhancedScalarImageToNeighbourhoodGreyLevelDifferenceFeaturesFilter<TImage, THistogramFrequencyContainer>
      ::GetMaskImage() const
    {
      if ( this->GetNumberOfInputs() < 2 )
      {
        return ITK_NULLPTR;
      }
      return static_cast< const ImageType *>( this->ProcessObject::GetInput( 1 ) );
    }

    template<typename TImage, typename THistogramFrequencyContainer>
    void
      EnhancedScalarImageToNeighbourhoodGreyLevelDifferenceFeaturesFilter<TImage, THistogramFrequencyContainer>
      ::SetInsidePixelValue( PixelType insidePixelValue )
    {
      itkDebugMacro( "setting InsidePixelValue to " << insidePixelValue );
      this->m_NeighbourhoodGreyLevelDifferenceMatrixGenerator->SetInsidePixelValue( insidePixelValue );
      this->Modified();
    }

    template<typename TImage, typename THistogramFrequencyContainer>
    void
      EnhancedScalarImageToNeighbourhoodGreyLevelDifferenceFeaturesFilter<TImage, THistogramFrequencyContainer>
      ::PrintSelf(std::ostream & os, Indent indent) const
    {
      Superclass::PrintSelf(os, indent);
      os << indent << "RequestedFeatures: "
        << this->GetRequestedFeatures() << std::endl;
      os << indent << "FeatureStandardDeviations: "
        << this->GetFeatureStandardDeviations() << std::endl;
      os << indent << "FastCalculations: "
        << this->GetFastCalculations() << std::endl;
      os << indent << "Offsets: " << this->GetOffsets() << std::endl;
      os << indent << "FeatureMeans: " << this->GetFeatureMeans() << std::endl;
    }
  } // end of namespace Statistics
} // end of namespace itk

#endif
