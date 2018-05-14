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
#ifndef __itkEnhancedHistogramToSizeZoneFeaturesFilter_hxx
#define __itkEnhancedHistogramToSizeZoneFeaturesFilter_hxx

#include "itkEnhancedHistogramToSizeZoneFeaturesFilter.h"

#include "itkNumericTraits.h"
#include "vnl/vnl_math.h"

namespace itk {
  namespace Statistics {
    //constructor
    template<typename THistogram>
    EnhancedHistogramToSizeZoneFeaturesFilter<THistogram>
      ::EnhancedHistogramToSizeZoneFeaturesFilter() :
    m_NumberOfVoxels(1)
    {
      this->ProcessObject::SetNumberOfRequiredInputs( 1 );

      // allocate the data objects for the outputs which are
      // just decorators real types
      for( unsigned int i = 0; i < 17; i++ )
      {
        this->ProcessObject::SetNthOutput( i, this->MakeOutput( i ) );
      }
    }

    template<typename THistogram>
    void
      EnhancedHistogramToSizeZoneFeaturesFilter< THistogram>
      ::SetInput( const HistogramType *histogram )
    {
      this->ProcessObject::SetNthInput( 0, const_cast<HistogramType*>( histogram ) );
    }

    template<typename THistogram>
    const typename
      EnhancedHistogramToSizeZoneFeaturesFilter<THistogram>::HistogramType *
      EnhancedHistogramToSizeZoneFeaturesFilter< THistogram>
      ::GetInput() const
    {
      if ( this->GetNumberOfInputs() < 1 )
      {
        return ITK_NULLPTR;
      }
      return itkDynamicCastInDebugMode<const HistogramType *>(this->ProcessObject::GetInput( 0 ) );
    }

    template<typename THistogram>
    typename
      EnhancedHistogramToSizeZoneFeaturesFilter<THistogram>::DataObjectPointer
      EnhancedHistogramToSizeZoneFeaturesFilter<THistogram>
      ::MakeOutput( DataObjectPointerArraySizeType itkNotUsed( idx ) )
    {
      return MeasurementObjectType::New().GetPointer();
    }

    template<typename THistogram>
    void
      EnhancedHistogramToSizeZoneFeaturesFilter< THistogram>::
      GenerateData( void )
    {
      const HistogramType * inputHistogram = this->GetInput();

      this->m_TotalNumberOfZones = static_cast<unsigned long>
        ( inputHistogram->GetTotalFrequency() );

      MeasurementType SmallZoneEmphasis = NumericTraits<MeasurementType>::ZeroValue();
      MeasurementType LargeZoneEmphasis = NumericTraits<MeasurementType>::ZeroValue();
      MeasurementType greyLevelNonuniformity = NumericTraits<MeasurementType>::ZeroValue();
      MeasurementType SizeZoneNonuniformity = NumericTraits<MeasurementType>::ZeroValue();
      MeasurementType lowGreyLevelZoneEmphasis = NumericTraits<MeasurementType>::ZeroValue();
      MeasurementType highGreyLevelZoneEmphasis = NumericTraits<MeasurementType>::ZeroValue();
      MeasurementType SmallZoneLowGreyLevelEmphasis = NumericTraits<MeasurementType>::ZeroValue();
      MeasurementType SmallZoneHighGreyLevelEmphasis = NumericTraits<MeasurementType>::ZeroValue();
      MeasurementType LargeZoneLowGreyLevelEmphasis = NumericTraits<MeasurementType>::ZeroValue();
      MeasurementType LargeZoneHighGreyLevelEmphasis = NumericTraits<MeasurementType>::ZeroValue();
      MeasurementType ZonePercentage = NumericTraits<MeasurementType>::ZeroValue();
      MeasurementType numberOfZones = NumericTraits<MeasurementType>::ZeroValue();
      //Added 15.07.2016
      MeasurementType greyLevelVariance = NumericTraits<MeasurementType>::ZeroValue();
      MeasurementType SizeZoneVariance = NumericTraits<MeasurementType>::ZeroValue();
      MeasurementType ZoneEntropy = NumericTraits<MeasurementType>::ZeroValue();

      //Added 09.09.2016
      MeasurementType greyLevelNonuniformityNormalized = NumericTraits<MeasurementType>::ZeroValue();
      MeasurementType SizeZoneNonuniformityNormalized = NumericTraits<MeasurementType>::ZeroValue();


      vnl_vector<double> greyLevelNonuniformityVector(
        inputHistogram->GetSize()[0], 0.0 );
      vnl_vector<double> SizeZoneNonuniformityVector(
        inputHistogram->GetSize()[1], 0.0 );

      typedef typename HistogramType::ConstIterator HistogramIterator;

      double mu_i = 0.0;
      double mu_j = 0.0;

      //Calculate the means.
      for ( HistogramIterator hit = inputHistogram->Begin();
        hit != inputHistogram->End(); ++hit )
      {
        MeasurementType frequency = hit.GetFrequency();
        if ( frequency == 0 )
        {
          continue;
        }
        MeasurementVectorType measurement = hit.GetMeasurementVector();
        IndexType index = hit.GetIndex();

        int value = floor(measurement[0] + 0.5);
        int count = measurement[1];

        double i = value;
        double j = count;

        double p_ij = frequency / m_TotalNumberOfZones;

        mu_i += i * p_ij;
        mu_j += j * p_ij;
      }

      //Calculate the other features.
      const double log2 = std::log(2.0);
      int totNumOfVoxelsUsed = 0;

      for ( HistogramIterator hit = inputHistogram->Begin();
        hit != inputHistogram->End(); ++hit )
      {
        MeasurementType frequency = hit.GetFrequency();
        if ( frequency == 0 )
        {
          continue;
        }
        MeasurementVectorType measurement = hit.GetMeasurementVector();
        IndexType index = hit.GetIndex();
        //      inputHistogram->GetIndex( hit.GetInstanceIdentifier() );

        int value = floor(measurement[0] + 0.5);
        int count = measurement[1];

        double i = value;
        double j = count;

        double i2 = static_cast<double>( i*i );
        double j2 = static_cast<double>( j*j );

        double p_ij = frequency / m_TotalNumberOfZones;

        greyLevelVariance += ((i - mu_i) * (i - mu_i) * p_ij);
        SizeZoneVariance += ((j - mu_j) * (j - mu_j) * p_ij);
        ZoneEntropy -= ( p_ij > 0.0001 ) ? p_ij *std::log(p_ij) / log2 : 0;

        // Traditional measures
        SmallZoneEmphasis += ( frequency / j2 );
        LargeZoneEmphasis += ( frequency * j2 );

        greyLevelNonuniformityVector[index[0]] += frequency;
        SizeZoneNonuniformityVector[index[1]] += frequency;

        // measures from Chu et al.
        lowGreyLevelZoneEmphasis += (i2 > 0.0001) ? ( frequency / i2 ) : 0;
        highGreyLevelZoneEmphasis += ( frequency * i2 );

        // measures from Dasarathy and Holder
        SmallZoneLowGreyLevelEmphasis += ((i2 * j2) > 0.0001) ? ( frequency / ( i2 * j2 ) ) : 0;
        SmallZoneHighGreyLevelEmphasis += (j2 > 0.00001) ? ( frequency * i2 / j2 ) : 0;
        LargeZoneLowGreyLevelEmphasis += (i2 > 0.00001) ? ( frequency * j2 / i2 ) : 0;
        LargeZoneHighGreyLevelEmphasis += ( frequency * i2 * j2 );

        totNumOfVoxelsUsed += (frequency);
      }
      greyLevelNonuniformity =
        greyLevelNonuniformityVector.squared_magnitude();
      SizeZoneNonuniformity =
        SizeZoneNonuniformityVector.squared_magnitude();

      // Normalize all measures by the total number of Zones

      m_TotalNumberOfZones = totNumOfVoxelsUsed;

      if (this->m_TotalNumberOfZones > 0)
      {
        SmallZoneEmphasis       /=
          static_cast<double>( this->m_TotalNumberOfZones );
        LargeZoneEmphasis        /=
          static_cast<double>( this->m_TotalNumberOfZones );
        greyLevelNonuniformity /=
          static_cast<double>( this->m_TotalNumberOfZones );
        SizeZoneNonuniformity /=
          static_cast<double>( this->m_TotalNumberOfZones );

        lowGreyLevelZoneEmphasis /=
          static_cast<double>( this->m_TotalNumberOfZones );
        highGreyLevelZoneEmphasis /=
          static_cast<double>( this->m_TotalNumberOfZones );

        SmallZoneLowGreyLevelEmphasis /=
          static_cast<double>( this->m_TotalNumberOfZones );
        SmallZoneHighGreyLevelEmphasis /=
          static_cast<double>( this->m_TotalNumberOfZones );
        LargeZoneLowGreyLevelEmphasis /=
          static_cast<double>( this->m_TotalNumberOfZones );
        LargeZoneHighGreyLevelEmphasis /=
          static_cast<double>( this->m_TotalNumberOfZones );
        ZonePercentage = static_cast<double>( this->m_TotalNumberOfZones ) / static_cast<double>( this->m_NumberOfVoxels );
        numberOfZones = static_cast<double>( this->m_TotalNumberOfZones ) ;

        greyLevelNonuniformityNormalized = greyLevelNonuniformity / static_cast<double>(this->m_TotalNumberOfZones);
        SizeZoneNonuniformityNormalized = SizeZoneNonuniformity / static_cast<double>(this->m_TotalNumberOfZones);

      } else {
        SmallZoneEmphasis      = 0;
        LargeZoneEmphasis      = 0;
        greyLevelNonuniformity = 0;
        SizeZoneNonuniformity  = 0;
        greyLevelNonuniformityNormalized = 0;
        SizeZoneNonuniformityNormalized  = 0;

        lowGreyLevelZoneEmphasis  = 0;
        highGreyLevelZoneEmphasis = 0;

        SmallZoneLowGreyLevelEmphasis = 0;
        SmallZoneHighGreyLevelEmphasis= 0;
        LargeZoneLowGreyLevelEmphasis = 0;
        LargeZoneHighGreyLevelEmphasis= 0;
        ZonePercentage = 0;
        numberOfZones = static_cast<double>( this->m_TotalNumberOfZones ) ;
      }

      MeasurementObjectType* SmallZoneEmphasisOutputObject =
        static_cast<MeasurementObjectType*>( this->ProcessObject::GetOutput( 0 ) );
      SmallZoneEmphasisOutputObject->Set( SmallZoneEmphasis );

      MeasurementObjectType* LargeZoneEmphasisOutputObject =
        static_cast<MeasurementObjectType*>( this->ProcessObject::GetOutput( 1 ) );
      LargeZoneEmphasisOutputObject->Set( LargeZoneEmphasis );

      MeasurementObjectType* greyLevelNonuniformityOutputObject =
        static_cast<MeasurementObjectType*>( this->ProcessObject::GetOutput( 2 ) );
      greyLevelNonuniformityOutputObject->Set( greyLevelNonuniformity );

      MeasurementObjectType* SizeZoneNonuniformityOutputObject =
        static_cast<MeasurementObjectType*>( this->ProcessObject::GetOutput( 3 ) );
      SizeZoneNonuniformityOutputObject->Set( SizeZoneNonuniformity );

      MeasurementObjectType* lowGreyLevelZoneEmphasisOutputObject =
        static_cast<MeasurementObjectType*>( this->ProcessObject::GetOutput( 4 ) );
      lowGreyLevelZoneEmphasisOutputObject->Set( lowGreyLevelZoneEmphasis );

      MeasurementObjectType* highGreyLevelZoneEmphasisOutputObject =
        static_cast<MeasurementObjectType*>( this->ProcessObject::GetOutput( 5 ) );
      highGreyLevelZoneEmphasisOutputObject->Set( highGreyLevelZoneEmphasis );

      MeasurementObjectType* SmallZoneLowGreyLevelEmphasisOutputObject =
        static_cast<MeasurementObjectType*>( this->ProcessObject::GetOutput( 6 ) );
      SmallZoneLowGreyLevelEmphasisOutputObject->Set( SmallZoneLowGreyLevelEmphasis );

      MeasurementObjectType* SmallZoneHighGreyLevelEmphasisOutputObject =
        static_cast<MeasurementObjectType*>( this->ProcessObject::GetOutput( 7 ) );
      SmallZoneHighGreyLevelEmphasisOutputObject->Set(
        SmallZoneHighGreyLevelEmphasis );

      MeasurementObjectType* LargeZoneLowGreyLevelEmphasisOutputObject =
        static_cast<MeasurementObjectType*>( this->ProcessObject::GetOutput( 8 ) );
      LargeZoneLowGreyLevelEmphasisOutputObject->Set( LargeZoneLowGreyLevelEmphasis );

      MeasurementObjectType* LargeZoneHighGreyLevelEmphasisOutputObject =
        static_cast<MeasurementObjectType*>( this->ProcessObject::GetOutput( 9 ) );
      LargeZoneHighGreyLevelEmphasisOutputObject->Set( LargeZoneHighGreyLevelEmphasis );

      MeasurementObjectType* ZonePercentagesOutputObject =
        static_cast<MeasurementObjectType*>( this->ProcessObject::GetOutput( 10 ) );
      ZonePercentagesOutputObject->Set( ZonePercentage );

      MeasurementObjectType* numberOfZonesOutputObject =
        static_cast<MeasurementObjectType*>( this->ProcessObject::GetOutput( 11 ) );
      numberOfZonesOutputObject->Set( numberOfZones );

      MeasurementObjectType* greyLevelVarianceOutputObject =
        static_cast<MeasurementObjectType*>( this->ProcessObject::GetOutput( 12 ) );
      greyLevelVarianceOutputObject->Set( greyLevelVariance );

      MeasurementObjectType* SizeZoneVarianceOutputObject =
        static_cast<MeasurementObjectType*>( this->ProcessObject::GetOutput( 13 ) );
      SizeZoneVarianceOutputObject->Set( SizeZoneVariance );

      MeasurementObjectType* ZoneEntropyOutputObject =
        static_cast<MeasurementObjectType*>( this->ProcessObject::GetOutput( 14 ) );
      ZoneEntropyOutputObject->Set( ZoneEntropy );

      MeasurementObjectType* greyLevelNonuniformityNormalizedOutputObject =
        static_cast<MeasurementObjectType*>( this->ProcessObject::GetOutput( 15 ) );
      greyLevelNonuniformityNormalizedOutputObject->Set( greyLevelNonuniformityNormalized );

      MeasurementObjectType* SizeZoneNonuniformityNormalizedOutputObject =
        static_cast<MeasurementObjectType*>( this->ProcessObject::GetOutput( 16 ) );
      SizeZoneNonuniformityNormalizedOutputObject->Set( SizeZoneNonuniformityNormalized );

    }

    template<typename THistogram>
    const
      typename EnhancedHistogramToSizeZoneFeaturesFilter<THistogram>::MeasurementObjectType*
      EnhancedHistogramToSizeZoneFeaturesFilter<THistogram>
      ::GetSmallZoneEmphasisOutput() const
    {
      return itkDynamicCastInDebugMode<const MeasurementObjectType*>(this->ProcessObject::GetOutput( 0 ) );
    }

    template<typename THistogram>
    const
      typename EnhancedHistogramToSizeZoneFeaturesFilter<THistogram>::MeasurementObjectType*
      EnhancedHistogramToSizeZoneFeaturesFilter<THistogram>
      ::GetLargeZoneEmphasisOutput() const
    {
      return itkDynamicCastInDebugMode<const MeasurementObjectType*>( this->ProcessObject::GetOutput( 1 ) );
    }

    template<typename THistogram>
    const
      typename EnhancedHistogramToSizeZoneFeaturesFilter<THistogram>::MeasurementObjectType*
      EnhancedHistogramToSizeZoneFeaturesFilter<THistogram>
      ::GetGreyLevelNonuniformityOutput() const
    {
      return itkDynamicCastInDebugMode<const MeasurementObjectType*>(this->ProcessObject::GetOutput( 2 ) );
    }

    template<typename THistogram>
    const
      typename EnhancedHistogramToSizeZoneFeaturesFilter<THistogram>::MeasurementObjectType*
      EnhancedHistogramToSizeZoneFeaturesFilter<THistogram>
      ::GetSizeZoneNonuniformityOutput() const
    {
      return itkDynamicCastInDebugMode<const MeasurementObjectType*>(this->ProcessObject::GetOutput( 3 ) );
    }

    template<typename THistogram>
    const
      typename EnhancedHistogramToSizeZoneFeaturesFilter<THistogram>::MeasurementObjectType*
      EnhancedHistogramToSizeZoneFeaturesFilter<THistogram>
      ::GetGreyLevelNonuniformityNormalizedOutput() const
    {
      return itkDynamicCastInDebugMode<const MeasurementObjectType*>(this->ProcessObject::GetOutput( 15 ) );
    }

    template<typename THistogram>
    const
      typename EnhancedHistogramToSizeZoneFeaturesFilter<THistogram>::MeasurementObjectType*
      EnhancedHistogramToSizeZoneFeaturesFilter<THistogram>
      ::GetSizeZoneNonuniformityNormalizedOutput() const
    {
      return itkDynamicCastInDebugMode<const MeasurementObjectType*>(this->ProcessObject::GetOutput( 16 ) );
    }

    template<typename THistogram>
    const
      typename EnhancedHistogramToSizeZoneFeaturesFilter<THistogram>::MeasurementObjectType*
      EnhancedHistogramToSizeZoneFeaturesFilter<THistogram>
      ::GetLowGreyLevelZoneEmphasisOutput() const
    {
      return itkDynamicCastInDebugMode<const MeasurementObjectType*>( this->ProcessObject::GetOutput( 4 ) );
    }

    template<typename THistogram>
    const
      typename EnhancedHistogramToSizeZoneFeaturesFilter<THistogram>::MeasurementObjectType*
      EnhancedHistogramToSizeZoneFeaturesFilter<THistogram>
      ::GetHighGreyLevelZoneEmphasisOutput() const
    {
      return itkDynamicCastInDebugMode<const MeasurementObjectType*>( this->ProcessObject::GetOutput( 5 ) );
    }

    template<typename THistogram>
    const
      typename EnhancedHistogramToSizeZoneFeaturesFilter<THistogram>::MeasurementObjectType*
      EnhancedHistogramToSizeZoneFeaturesFilter<THistogram>
      ::GetSmallZoneLowGreyLevelEmphasisOutput() const
    {
      return itkDynamicCastInDebugMode<const MeasurementObjectType*>( this->ProcessObject::GetOutput( 6 ) );
    }

    template<typename THistogram>
    const
      typename EnhancedHistogramToSizeZoneFeaturesFilter<THistogram>::MeasurementObjectType*
      EnhancedHistogramToSizeZoneFeaturesFilter<THistogram>
      ::GetSmallZoneHighGreyLevelEmphasisOutput() const
    {
      return itkDynamicCastInDebugMode<const MeasurementObjectType*>( this->ProcessObject::GetOutput( 7 ) );
    }

    template<typename THistogram>
    const
      typename EnhancedHistogramToSizeZoneFeaturesFilter<THistogram>::MeasurementObjectType*
      EnhancedHistogramToSizeZoneFeaturesFilter<THistogram>
      ::GetLargeZoneLowGreyLevelEmphasisOutput() const
    {
      return itkDynamicCastInDebugMode<const MeasurementObjectType*>( this->ProcessObject::GetOutput( 8 ) );
    }

    template<typename THistogram>
    const
      typename EnhancedHistogramToSizeZoneFeaturesFilter<THistogram>::MeasurementObjectType*
      EnhancedHistogramToSizeZoneFeaturesFilter<THistogram>
      ::GetLargeZoneHighGreyLevelEmphasisOutput() const
    {
      return itkDynamicCastInDebugMode<const MeasurementObjectType*>( this->ProcessObject::GetOutput( 9 ) );
    }

    template<typename THistogram>
    const
      typename EnhancedHistogramToSizeZoneFeaturesFilter<THistogram>::MeasurementObjectType*
      EnhancedHistogramToSizeZoneFeaturesFilter<THistogram>
      ::GetZonePercentageOutput() const
    {
      return itkDynamicCastInDebugMode<const MeasurementObjectType*>( this->ProcessObject::GetOutput( 10 ) );
    }

    template<typename THistogram>
    const
      typename EnhancedHistogramToSizeZoneFeaturesFilter<THistogram>::MeasurementObjectType*
      EnhancedHistogramToSizeZoneFeaturesFilter<THistogram>
      ::GetNumberOfZonesOutput() const
    {
      return itkDynamicCastInDebugMode<const MeasurementObjectType*>( this->ProcessObject::GetOutput( 11 ) );
    }

    template<typename THistogram>
    const
      typename EnhancedHistogramToSizeZoneFeaturesFilter<THistogram>::MeasurementObjectType*
      EnhancedHistogramToSizeZoneFeaturesFilter<THistogram>
      ::GetGreyLevelVarianceOutput() const
    {
      return itkDynamicCastInDebugMode<const MeasurementObjectType*>( this->ProcessObject::GetOutput( 12 ) );
    }

    template<typename THistogram>
    const
      typename EnhancedHistogramToSizeZoneFeaturesFilter<THistogram>::MeasurementObjectType*
      EnhancedHistogramToSizeZoneFeaturesFilter<THistogram>
      ::GetSizeZoneVarianceOutput() const
    {
      return itkDynamicCastInDebugMode<const MeasurementObjectType*>( this->ProcessObject::GetOutput( 13 ) );
    }

    template<typename THistogram>
    const
      typename EnhancedHistogramToSizeZoneFeaturesFilter<THistogram>::MeasurementObjectType*
      EnhancedHistogramToSizeZoneFeaturesFilter<THistogram>
      ::GetZoneEntropyOutput() const
    {
      return itkDynamicCastInDebugMode<const MeasurementObjectType*>( this->ProcessObject::GetOutput( 14 ) );
    }

    template<typename THistogram>
    typename EnhancedHistogramToSizeZoneFeaturesFilter<THistogram>::MeasurementType
      EnhancedHistogramToSizeZoneFeaturesFilter<THistogram>
      ::GetSmallZoneEmphasis() const
    {
      return this->GetSmallZoneEmphasisOutput()->Get();
    }

    template<typename THistogram>
    typename EnhancedHistogramToSizeZoneFeaturesFilter<THistogram>::MeasurementType
      EnhancedHistogramToSizeZoneFeaturesFilter<THistogram>
      ::GetLargeZoneEmphasis() const
    {
      return this->GetLargeZoneEmphasisOutput()->Get();
    }

    template<typename THistogram>
    typename EnhancedHistogramToSizeZoneFeaturesFilter<THistogram>::MeasurementType
      EnhancedHistogramToSizeZoneFeaturesFilter<THistogram>
      ::GetGreyLevelNonuniformity() const
    {
      return this->GetGreyLevelNonuniformityOutput()->Get();
    }

    template<typename THistogram>
    typename EnhancedHistogramToSizeZoneFeaturesFilter<THistogram>::MeasurementType
      EnhancedHistogramToSizeZoneFeaturesFilter<THistogram>
      ::GetGreyLevelNonuniformityNormalized() const
    {
      return this->GetGreyLevelNonuniformityNormalizedOutput()->Get();
    }

    template<typename THistogram>
    typename EnhancedHistogramToSizeZoneFeaturesFilter<THistogram>::MeasurementType
      EnhancedHistogramToSizeZoneFeaturesFilter<THistogram>
      ::GetSizeZoneNonuniformity() const
    {
      return this->GetSizeZoneNonuniformityOutput()->Get();
    }

    template<typename THistogram>
    typename EnhancedHistogramToSizeZoneFeaturesFilter<THistogram>::MeasurementType
      EnhancedHistogramToSizeZoneFeaturesFilter<THistogram>
      ::GetSizeZoneNonuniformityNormalized() const
    {
      return this->GetSizeZoneNonuniformityNormalizedOutput()->Get();
    }

    template<typename THistogram>
    typename EnhancedHistogramToSizeZoneFeaturesFilter<THistogram>::MeasurementType
      EnhancedHistogramToSizeZoneFeaturesFilter<THistogram>
      ::GetLowGreyLevelZoneEmphasis() const
    {
      return this->GetLowGreyLevelZoneEmphasisOutput()->Get();
    }
    template<typename THistogram>
    typename EnhancedHistogramToSizeZoneFeaturesFilter<THistogram>::MeasurementType
      EnhancedHistogramToSizeZoneFeaturesFilter<THistogram>
      ::GetHighGreyLevelZoneEmphasis() const
    {
      return this->GetHighGreyLevelZoneEmphasisOutput()->Get();
    }
    template<typename THistogram>
    typename EnhancedHistogramToSizeZoneFeaturesFilter<THistogram>::MeasurementType
      EnhancedHistogramToSizeZoneFeaturesFilter<THistogram>
      ::GetSmallZoneLowGreyLevelEmphasis() const
    {
      return this->GetSmallZoneLowGreyLevelEmphasisOutput()->Get();
    }
    template<typename THistogram>
    typename EnhancedHistogramToSizeZoneFeaturesFilter<THistogram>::MeasurementType
      EnhancedHistogramToSizeZoneFeaturesFilter<THistogram>
      ::GetSmallZoneHighGreyLevelEmphasis() const
    {
      return this->GetSmallZoneHighGreyLevelEmphasisOutput()->Get();
    }
    template<typename THistogram>
    typename EnhancedHistogramToSizeZoneFeaturesFilter<THistogram>::MeasurementType
      EnhancedHistogramToSizeZoneFeaturesFilter<THistogram>
      ::GetLargeZoneLowGreyLevelEmphasis() const
    {
      return this->GetLargeZoneLowGreyLevelEmphasisOutput()->Get();
    }
    template<typename THistogram>
    typename EnhancedHistogramToSizeZoneFeaturesFilter<THistogram>::MeasurementType
      EnhancedHistogramToSizeZoneFeaturesFilter<THistogram>
      ::GetLargeZoneHighGreyLevelEmphasis() const
    {
      return this->GetLargeZoneHighGreyLevelEmphasisOutput()->Get();
    }
    template<typename THistogram>
    typename EnhancedHistogramToSizeZoneFeaturesFilter<THistogram>::MeasurementType
      EnhancedHistogramToSizeZoneFeaturesFilter<THistogram>
      ::GetZonePercentage() const
    {
      return this->GetZonePercentageOutput()->Get();
    }
    template<typename THistogram>
    typename EnhancedHistogramToSizeZoneFeaturesFilter<THistogram>::MeasurementType
      EnhancedHistogramToSizeZoneFeaturesFilter<THistogram>
      ::GetNumberOfZones() const
    {
      return this->GetNumberOfZonesOutput()->Get();
    }
    template<typename THistogram>
    typename EnhancedHistogramToSizeZoneFeaturesFilter<THistogram>::MeasurementType
      EnhancedHistogramToSizeZoneFeaturesFilter<THistogram>
      ::GetGreyLevelVariance() const
    {
      return this->GetGreyLevelVarianceOutput()->Get();
    }
    template<typename THistogram>
    typename EnhancedHistogramToSizeZoneFeaturesFilter<THistogram>::MeasurementType
      EnhancedHistogramToSizeZoneFeaturesFilter<THistogram>
      ::GetSizeZoneVariance() const
    {
      return this->GetSizeZoneVarianceOutput()->Get();
    }
    template<typename THistogram>
    typename EnhancedHistogramToSizeZoneFeaturesFilter<THistogram>::MeasurementType
      EnhancedHistogramToSizeZoneFeaturesFilter<THistogram>
      ::GetZoneEntropy() const
    {
      return this->GetZoneEntropyOutput()->Get();
    }


    template<typename THistogram>
    typename EnhancedHistogramToSizeZoneFeaturesFilter< THistogram>::MeasurementType
      EnhancedHistogramToSizeZoneFeaturesFilter<THistogram>
      ::GetFeature( SizeZoneFeatureName feature )
    {
      switch( feature )
      {
      case SmallZoneEmphasis:
        return this->GetSmallZoneEmphasis();
      case LargeZoneEmphasis:
        return this->GetLargeZoneEmphasis();
      case GreyLevelNonuniformity:
        return this->GetGreyLevelNonuniformity();
      case GreyLevelNonuniformityNormalized:
        return this->GetGreyLevelNonuniformityNormalized();
      case SizeZoneNonuniformity:
        return this->GetSizeZoneNonuniformity();
      case SizeZoneNonuniformityNormalized:
        return this->GetSizeZoneNonuniformityNormalized();
      case LowGreyLevelZoneEmphasis:
        return this->GetLowGreyLevelZoneEmphasis();
      case HighGreyLevelZoneEmphasis:
        return this->GetHighGreyLevelZoneEmphasis();
      case SmallZoneLowGreyLevelEmphasis:
        return this->GetSmallZoneLowGreyLevelEmphasis();
      case SmallZoneHighGreyLevelEmphasis:
        return this->GetSmallZoneHighGreyLevelEmphasis();
      case LargeZoneLowGreyLevelEmphasis:
        return this->GetLargeZoneLowGreyLevelEmphasis();
      case LargeZoneHighGreyLevelEmphasis:
        return this->GetLargeZoneHighGreyLevelEmphasis();
      case ZonePercentage:
        return this->GetZonePercentage();
      case GreyLevelVariance:
        return this->GetGreyLevelVariance();
      case SizeZoneVariance:
        return this->GetSizeZoneVariance();
      case ZoneEntropy:
        return this->GetZoneEntropy();
      default:
        return 0;
      }
    }

    template< typename THistogram>
    void
      EnhancedHistogramToSizeZoneFeaturesFilter< THistogram>::
      PrintSelf(std::ostream& os, Indent indent) const
    {
      Superclass::PrintSelf( os,indent );
    }
  } // end of namespace Statistics
} // end of namespace itk

#endif
