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
#ifndef __itkEnhancedHistogramToRunLengthFeaturesFilter_hxx
#define __itkEnhancedHistogramToRunLengthFeaturesFilter_hxx

#include "itkEnhancedHistogramToRunLengthFeaturesFilter.h"

#include "itkNumericTraits.h"
#include "vnl/vnl_math.h"

namespace itk {
  namespace Statistics {
    //constructor
    template<typename THistogram>
    EnhancedHistogramToRunLengthFeaturesFilter<THistogram>
      ::EnhancedHistogramToRunLengthFeaturesFilter() :
    m_NumberOfVoxels(1)
    {
      this->ProcessObject::SetNumberOfRequiredInputs( 1 );

      // allocate the data objects for the outputs which are
      // just decorators real types
      for( unsigned int i = 0; i < 12; i++ )
      {
        this->ProcessObject::SetNthOutput( i, this->MakeOutput( i ) );
      }
    }

    template<typename THistogram>
    void
      EnhancedHistogramToRunLengthFeaturesFilter< THistogram>
      ::SetInput( const HistogramType *histogram )
    {
      this->ProcessObject::SetNthInput( 0, const_cast<HistogramType*>( histogram ) );
    }

    template<typename THistogram>
    const typename
      EnhancedHistogramToRunLengthFeaturesFilter<THistogram>::HistogramType *
      EnhancedHistogramToRunLengthFeaturesFilter< THistogram>
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
      EnhancedHistogramToRunLengthFeaturesFilter<THistogram>::DataObjectPointer
      EnhancedHistogramToRunLengthFeaturesFilter<THistogram>
      ::MakeOutput( DataObjectPointerArraySizeType itkNotUsed( idx ) )
    {
      return MeasurementObjectType::New().GetPointer();
    }

    template<typename THistogram>
    void
      EnhancedHistogramToRunLengthFeaturesFilter< THistogram>::
      GenerateData( void )
    {
      const HistogramType * inputHistogram = this->GetInput();

      this->m_TotalNumberOfRuns = static_cast<unsigned long>
        ( inputHistogram->GetTotalFrequency() );

      MeasurementType shortRunEmphasis = NumericTraits<MeasurementType>::ZeroValue();
      MeasurementType longRunEmphasis = NumericTraits<MeasurementType>::ZeroValue();
      MeasurementType greyLevelNonuniformity = NumericTraits<MeasurementType>::ZeroValue();
      MeasurementType runLengthNonuniformity = NumericTraits<MeasurementType>::ZeroValue();
      MeasurementType lowGreyLevelRunEmphasis = NumericTraits<MeasurementType>::ZeroValue();
      MeasurementType highGreyLevelRunEmphasis = NumericTraits<MeasurementType>::ZeroValue();
      MeasurementType shortRunLowGreyLevelEmphasis = NumericTraits<MeasurementType>::ZeroValue();
      MeasurementType shortRunHighGreyLevelEmphasis = NumericTraits<MeasurementType>::ZeroValue();
      MeasurementType longRunLowGreyLevelEmphasis = NumericTraits<MeasurementType>::ZeroValue();
      MeasurementType longRunHighGreyLevelEmphasis = NumericTraits<MeasurementType>::ZeroValue();
      MeasurementType runPercentage = NumericTraits<MeasurementType>::ZeroValue();
      MeasurementType numberOfRuns = NumericTraits<MeasurementType>::ZeroValue();

      vnl_vector<double> greyLevelNonuniformityVector(
        inputHistogram->GetSize()[0], 0.0 );
      vnl_vector<double> runLengthNonuniformityVector(
        inputHistogram->GetSize()[1], 0.0 );

      typedef typename HistogramType::ConstIterator HistogramIterator;
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

        double i2 = static_cast<double>( ( index[0] + 1 ) * ( index[0] + 1 ) );
        double j2 = static_cast<double>( ( index[1] + 1 ) * ( index[1] + 1 ) );

        // Traditional measures
        shortRunEmphasis += ( frequency / j2 );
        longRunEmphasis += ( frequency * j2 );

        greyLevelNonuniformityVector[index[0]] += frequency;
        runLengthNonuniformityVector[index[1]] += frequency;

        // measures from Chu et al.
        lowGreyLevelRunEmphasis += ( frequency / i2 );
        highGreyLevelRunEmphasis += ( frequency * i2 );

        // measures from Dasarathy and Holder
        shortRunLowGreyLevelEmphasis += ( frequency / ( i2 * j2 ) );
        shortRunHighGreyLevelEmphasis += ( frequency * i2 / j2 );
        longRunLowGreyLevelEmphasis += ( frequency * j2 / i2 );
        longRunHighGreyLevelEmphasis += ( frequency * i2 * j2 );
      }
      greyLevelNonuniformity =
        greyLevelNonuniformityVector.squared_magnitude();
      runLengthNonuniformity =
        runLengthNonuniformityVector.squared_magnitude();

      // Normalize all measures by the total number of runs

      if (this->m_TotalNumberOfRuns > 0)
      {
        shortRunEmphasis       /=
          static_cast<double>( this->m_TotalNumberOfRuns );
        longRunEmphasis        /=
          static_cast<double>( this->m_TotalNumberOfRuns );
        greyLevelNonuniformity /=
          static_cast<double>( this->m_TotalNumberOfRuns );
        runLengthNonuniformity /=
          static_cast<double>( this->m_TotalNumberOfRuns );

        lowGreyLevelRunEmphasis /=
          static_cast<double>( this->m_TotalNumberOfRuns );
        highGreyLevelRunEmphasis /=
          static_cast<double>( this->m_TotalNumberOfRuns );

        shortRunLowGreyLevelEmphasis /=
          static_cast<double>( this->m_TotalNumberOfRuns );
        shortRunHighGreyLevelEmphasis /=
          static_cast<double>( this->m_TotalNumberOfRuns );
        longRunLowGreyLevelEmphasis /=
          static_cast<double>( this->m_TotalNumberOfRuns );
        longRunHighGreyLevelEmphasis /=
          static_cast<double>( this->m_TotalNumberOfRuns );
        runPercentage = static_cast<double>( this->m_TotalNumberOfRuns ) / static_cast<double>( this->m_NumberOfVoxels );
        numberOfRuns = static_cast<double>( this->m_TotalNumberOfRuns ) ;
      } else {
        shortRunEmphasis        = 0;
        longRunEmphasis         = 0;
        greyLevelNonuniformity  = 0;
        runLengthNonuniformity= 0;

        lowGreyLevelRunEmphasis  = 0;
        highGreyLevelRunEmphasis = 0;

        shortRunLowGreyLevelEmphasis = 0;
        shortRunHighGreyLevelEmphasis= 0;
        longRunLowGreyLevelEmphasis  = 0;
        longRunHighGreyLevelEmphasis = 0;
        runPercentage = 0;
        numberOfRuns = static_cast<double>( this->m_TotalNumberOfRuns ) ;
      }

      MeasurementObjectType* shortRunEmphasisOutputObject =
        static_cast<MeasurementObjectType*>( this->ProcessObject::GetOutput( 0 ) );
      shortRunEmphasisOutputObject->Set( shortRunEmphasis );

      MeasurementObjectType* longRunEmphasisOutputObject =
        static_cast<MeasurementObjectType*>( this->ProcessObject::GetOutput( 1 ) );
      longRunEmphasisOutputObject->Set( longRunEmphasis );

      MeasurementObjectType* greyLevelNonuniformityOutputObject =
        static_cast<MeasurementObjectType*>( this->ProcessObject::GetOutput( 2 ) );
      greyLevelNonuniformityOutputObject->Set( greyLevelNonuniformity );

      MeasurementObjectType* runLengthNonuniformityOutputObject =
        static_cast<MeasurementObjectType*>( this->ProcessObject::GetOutput( 3 ) );
      runLengthNonuniformityOutputObject->Set( runLengthNonuniformity );

      MeasurementObjectType* lowGreyLevelRunEmphasisOutputObject =
        static_cast<MeasurementObjectType*>( this->ProcessObject::GetOutput( 4 ) );
      lowGreyLevelRunEmphasisOutputObject->Set( lowGreyLevelRunEmphasis );

      MeasurementObjectType* highGreyLevelRunEmphasisOutputObject =
        static_cast<MeasurementObjectType*>( this->ProcessObject::GetOutput( 5 ) );
      highGreyLevelRunEmphasisOutputObject->Set( highGreyLevelRunEmphasis );

      MeasurementObjectType* shortRunLowGreyLevelEmphasisOutputObject =
        static_cast<MeasurementObjectType*>( this->ProcessObject::GetOutput( 6 ) );
      shortRunLowGreyLevelEmphasisOutputObject->Set( shortRunLowGreyLevelEmphasis );

      MeasurementObjectType* shortRunHighGreyLevelEmphasisOutputObject =
        static_cast<MeasurementObjectType*>( this->ProcessObject::GetOutput( 7 ) );
      shortRunHighGreyLevelEmphasisOutputObject->Set(
        shortRunHighGreyLevelEmphasis );

      MeasurementObjectType* longRunLowGreyLevelEmphasisOutputObject =
        static_cast<MeasurementObjectType*>( this->ProcessObject::GetOutput( 8 ) );
      longRunLowGreyLevelEmphasisOutputObject->Set( longRunLowGreyLevelEmphasis );

      MeasurementObjectType* longRunHighGreyLevelEmphasisOutputObject =
        static_cast<MeasurementObjectType*>( this->ProcessObject::GetOutput( 9 ) );
      longRunHighGreyLevelEmphasisOutputObject->Set( longRunHighGreyLevelEmphasis );

      MeasurementObjectType* runPercentagesOutputObject =
        static_cast<MeasurementObjectType*>( this->ProcessObject::GetOutput( 10 ) );
      runPercentagesOutputObject->Set( runPercentage );

      MeasurementObjectType* numberOfRunsOutputObject =
        static_cast<MeasurementObjectType*>( this->ProcessObject::GetOutput( 11 ) );
      numberOfRunsOutputObject->Set( numberOfRuns );
    }

    template<typename THistogram>
    const
      typename EnhancedHistogramToRunLengthFeaturesFilter<THistogram>::MeasurementObjectType*
      EnhancedHistogramToRunLengthFeaturesFilter<THistogram>
      ::GetShortRunEmphasisOutput() const
    {
      return itkDynamicCastInDebugMode<const MeasurementObjectType*>(this->ProcessObject::GetOutput( 0 ) );
    }

    template<typename THistogram>
    const
      typename EnhancedHistogramToRunLengthFeaturesFilter<THistogram>::MeasurementObjectType*
      EnhancedHistogramToRunLengthFeaturesFilter<THistogram>
      ::GetLongRunEmphasisOutput() const
    {
      return itkDynamicCastInDebugMode<const MeasurementObjectType*>( this->ProcessObject::GetOutput( 1 ) );
    }

    template<typename THistogram>
    const
      typename EnhancedHistogramToRunLengthFeaturesFilter<THistogram>::MeasurementObjectType*
      EnhancedHistogramToRunLengthFeaturesFilter<THistogram>
      ::GetGreyLevelNonuniformityOutput() const
    {
      return itkDynamicCastInDebugMode<const MeasurementObjectType*>(this->ProcessObject::GetOutput( 2 ) );
    }

    template<typename THistogram>
    const
      typename EnhancedHistogramToRunLengthFeaturesFilter<THistogram>::MeasurementObjectType*
      EnhancedHistogramToRunLengthFeaturesFilter<THistogram>
      ::GetRunLengthNonuniformityOutput() const
    {
      return itkDynamicCastInDebugMode<const MeasurementObjectType*>(this->ProcessObject::GetOutput( 3 ) );
    }

    template<typename THistogram>
    const
      typename EnhancedHistogramToRunLengthFeaturesFilter<THistogram>::MeasurementObjectType*
      EnhancedHistogramToRunLengthFeaturesFilter<THistogram>
      ::GetLowGreyLevelRunEmphasisOutput() const
    {
      return itkDynamicCastInDebugMode<const MeasurementObjectType*>( this->ProcessObject::GetOutput( 4 ) );
    }

    template<typename THistogram>
    const
      typename EnhancedHistogramToRunLengthFeaturesFilter<THistogram>::MeasurementObjectType*
      EnhancedHistogramToRunLengthFeaturesFilter<THistogram>
      ::GetHighGreyLevelRunEmphasisOutput() const
    {
      return itkDynamicCastInDebugMode<const MeasurementObjectType*>( this->ProcessObject::GetOutput( 5 ) );
    }

    template<typename THistogram>
    const
      typename EnhancedHistogramToRunLengthFeaturesFilter<THistogram>::MeasurementObjectType*
      EnhancedHistogramToRunLengthFeaturesFilter<THistogram>
      ::GetShortRunLowGreyLevelEmphasisOutput() const
    {
      return itkDynamicCastInDebugMode<const MeasurementObjectType*>( this->ProcessObject::GetOutput( 6 ) );
    }

    template<typename THistogram>
    const
      typename EnhancedHistogramToRunLengthFeaturesFilter<THistogram>::MeasurementObjectType*
      EnhancedHistogramToRunLengthFeaturesFilter<THistogram>
      ::GetShortRunHighGreyLevelEmphasisOutput() const
    {
      return itkDynamicCastInDebugMode<const MeasurementObjectType*>( this->ProcessObject::GetOutput( 7 ) );
    }

    template<typename THistogram>
    const
      typename EnhancedHistogramToRunLengthFeaturesFilter<THistogram>::MeasurementObjectType*
      EnhancedHistogramToRunLengthFeaturesFilter<THistogram>
      ::GetLongRunLowGreyLevelEmphasisOutput() const
    {
      return itkDynamicCastInDebugMode<const MeasurementObjectType*>( this->ProcessObject::GetOutput( 8 ) );
    }

    template<typename THistogram>
    const
      typename EnhancedHistogramToRunLengthFeaturesFilter<THistogram>::MeasurementObjectType*
      EnhancedHistogramToRunLengthFeaturesFilter<THistogram>
      ::GetLongRunHighGreyLevelEmphasisOutput() const
    {
      return itkDynamicCastInDebugMode<const MeasurementObjectType*>( this->ProcessObject::GetOutput( 9 ) );
    }

    template<typename THistogram>
    const
      typename EnhancedHistogramToRunLengthFeaturesFilter<THistogram>::MeasurementObjectType*
      EnhancedHistogramToRunLengthFeaturesFilter<THistogram>
      ::GetRunPercentageOutput() const
    {
      return itkDynamicCastInDebugMode<const MeasurementObjectType*>( this->ProcessObject::GetOutput( 10 ) );
    }

    template<typename THistogram>
    const
      typename EnhancedHistogramToRunLengthFeaturesFilter<THistogram>::MeasurementObjectType*
      EnhancedHistogramToRunLengthFeaturesFilter<THistogram>
      ::GetNumberOfRunsOutput() const
    {
      return itkDynamicCastInDebugMode<const MeasurementObjectType*>( this->ProcessObject::GetOutput( 11 ) );
    }

    template<typename THistogram>
    typename EnhancedHistogramToRunLengthFeaturesFilter<THistogram>::MeasurementType
      EnhancedHistogramToRunLengthFeaturesFilter<THistogram>
      ::GetShortRunEmphasis() const
    {
      return this->GetShortRunEmphasisOutput()->Get();
    }

    template<typename THistogram>
    typename EnhancedHistogramToRunLengthFeaturesFilter<THistogram>::MeasurementType
      EnhancedHistogramToRunLengthFeaturesFilter<THistogram>
      ::GetLongRunEmphasis() const
    {
      return this->GetLongRunEmphasisOutput()->Get();
    }

    template<typename THistogram>
    typename EnhancedHistogramToRunLengthFeaturesFilter<THistogram>::MeasurementType
      EnhancedHistogramToRunLengthFeaturesFilter<THistogram>
      ::GetGreyLevelNonuniformity() const
    {
      return this->GetGreyLevelNonuniformityOutput()->Get();
    }

    template<typename THistogram>
    typename EnhancedHistogramToRunLengthFeaturesFilter<THistogram>::MeasurementType
      EnhancedHistogramToRunLengthFeaturesFilter<THistogram>
      ::GetRunLengthNonuniformity() const
    {
      return this->GetRunLengthNonuniformityOutput()->Get();
    }

    template<typename THistogram>
    typename EnhancedHistogramToRunLengthFeaturesFilter<THistogram>::MeasurementType
      EnhancedHistogramToRunLengthFeaturesFilter<THistogram>
      ::GetLowGreyLevelRunEmphasis() const
    {
      return this->GetLowGreyLevelRunEmphasisOutput()->Get();
    }
    template<typename THistogram>
    typename EnhancedHistogramToRunLengthFeaturesFilter<THistogram>::MeasurementType
      EnhancedHistogramToRunLengthFeaturesFilter<THistogram>
      ::GetHighGreyLevelRunEmphasis() const
    {
      return this->GetHighGreyLevelRunEmphasisOutput()->Get();
    }
    template<typename THistogram>
    typename EnhancedHistogramToRunLengthFeaturesFilter<THistogram>::MeasurementType
      EnhancedHistogramToRunLengthFeaturesFilter<THistogram>
      ::GetShortRunLowGreyLevelEmphasis() const
    {
      return this->GetShortRunLowGreyLevelEmphasisOutput()->Get();
    }
    template<typename THistogram>
    typename EnhancedHistogramToRunLengthFeaturesFilter<THistogram>::MeasurementType
      EnhancedHistogramToRunLengthFeaturesFilter<THistogram>
      ::GetShortRunHighGreyLevelEmphasis() const
    {
      return this->GetShortRunHighGreyLevelEmphasisOutput()->Get();
    }
    template<typename THistogram>
    typename EnhancedHistogramToRunLengthFeaturesFilter<THistogram>::MeasurementType
      EnhancedHistogramToRunLengthFeaturesFilter<THistogram>
      ::GetLongRunLowGreyLevelEmphasis() const
    {
      return this->GetLongRunLowGreyLevelEmphasisOutput()->Get();
    }
    template<typename THistogram>
    typename EnhancedHistogramToRunLengthFeaturesFilter<THistogram>::MeasurementType
      EnhancedHistogramToRunLengthFeaturesFilter<THistogram>
      ::GetLongRunHighGreyLevelEmphasis() const
    {
      return this->GetLongRunHighGreyLevelEmphasisOutput()->Get();
    }
    template<typename THistogram>
    typename EnhancedHistogramToRunLengthFeaturesFilter<THistogram>::MeasurementType
      EnhancedHistogramToRunLengthFeaturesFilter<THistogram>
      ::GetRunPercentage() const
    {
      return this->GetRunPercentageOutput()->Get();
    }
    template<typename THistogram>
    typename EnhancedHistogramToRunLengthFeaturesFilter<THistogram>::MeasurementType
      EnhancedHistogramToRunLengthFeaturesFilter<THistogram>
      ::GetNumberOfRuns() const
    {
      return this->GetNumberOfRunsOutput()->Get();
    }

    template<typename THistogram>
    typename EnhancedHistogramToRunLengthFeaturesFilter< THistogram>::MeasurementType
      EnhancedHistogramToRunLengthFeaturesFilter<THistogram>
      ::GetFeature( RunLengthFeatureName feature )
    {
      switch( feature )
      {
      case ShortRunEmphasis:
        return this->GetShortRunEmphasis();
      case LongRunEmphasis:
        return this->GetLongRunEmphasis();
      case GreyLevelNonuniformity:
        return this->GetGreyLevelNonuniformity();
      case RunLengthNonuniformity:
        return this->GetRunLengthNonuniformity();
      case LowGreyLevelRunEmphasis:
        return this->GetLowGreyLevelRunEmphasis();
      case HighGreyLevelRunEmphasis:
        return this->GetHighGreyLevelRunEmphasis();
      case ShortRunLowGreyLevelEmphasis:
        return this->GetShortRunLowGreyLevelEmphasis();
      case ShortRunHighGreyLevelEmphasis:
        return this->GetShortRunHighGreyLevelEmphasis();
      case LongRunLowGreyLevelEmphasis:
        return this->GetLongRunLowGreyLevelEmphasis();
      case LongRunHighGreyLevelEmphasis:
        return this->GetLongRunHighGreyLevelEmphasis();
      case RunPercentage:
        return this->GetRunPercentage();
      case NumberOfRuns:
        return this->GetNumberOfRuns();
      default:
        return 0;
      }
    }

    template< typename THistogram>
    void
      EnhancedHistogramToRunLengthFeaturesFilter< THistogram>::
      PrintSelf(std::ostream& os, Indent indent) const
    {
      Superclass::PrintSelf( os,indent );
    }
  } // end of namespace Statistics
} // end of namespace itk

#endif