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
#ifndef __itkEnhancedHistogramToNeighbourhoodGreyLevelDifferenceFeaturesFilter_hxx
#define __itkEnhancedHistogramToNeighbourhoodGreyLevelDifferenceFeaturesFilter_hxx

#include "itkEnhancedHistogramToNeighbourhoodGreyLevelDifferenceFeaturesFilter.h"

#include "itkNumericTraits.h"
#include "vnl/vnl_math.h"

namespace itk {
namespace Statistics {
//constructor
template<typename THistogram>
EnhancedHistogramToNeighbourhoodGreyLevelDifferenceFeaturesFilter<THistogram>
::EnhancedHistogramToNeighbourhoodGreyLevelDifferenceFeaturesFilter() :
  m_NumberOfVoxels(1)
{
  this->ProcessObject::SetNumberOfRequiredInputs( 1 );

  // allocate the data objects for the outputs which are
  // just decorators real types
  for( unsigned int i = 0; i < 5; i++ )
  {
    this->ProcessObject::SetNthOutput( i, this->MakeOutput( i ) );
  }
}

template<typename THistogram>
void
EnhancedHistogramToNeighbourhoodGreyLevelDifferenceFeaturesFilter< THistogram>
::SetInput(const HistogramType *histogram )
{
  this->ProcessObject::SetNthInput( 0, const_cast<HistogramType*>( histogram ) );
}


template<typename THistogram>
const typename
EnhancedHistogramToNeighbourhoodGreyLevelDifferenceFeaturesFilter<THistogram>::HistogramType *
EnhancedHistogramToNeighbourhoodGreyLevelDifferenceFeaturesFilter< THistogram>
::GetInput() const
{
  if ( this->GetNumberOfInputs() < 1 )
  {
    return ITK_NULLPTR;
  }
  return itkDynamicCastInDebugMode<const HistogramType *>(this->ProcessObject::GetInput(0) );
}

template<typename THistogram>
typename
EnhancedHistogramToNeighbourhoodGreyLevelDifferenceFeaturesFilter<THistogram>::DataObjectPointer
EnhancedHistogramToNeighbourhoodGreyLevelDifferenceFeaturesFilter<THistogram>
::MakeOutput( DataObjectPointerArraySizeType itkNotUsed( idx ) )
{
  return MeasurementObjectType::New().GetPointer();
}

template<typename THistogram>
void
EnhancedHistogramToNeighbourhoodGreyLevelDifferenceFeaturesFilter< THistogram>::
GenerateData( void )
{
  const HistogramType * inputHistogram = this->GetInput(); //The nis

  this->m_TotalNumberOfValidVoxels = static_cast<unsigned long>
      ( inputHistogram->GetTotalFrequency() );

  MeasurementType coarseness  = NumericTraits<MeasurementType>::ZeroValue();
  MeasurementType contrast    = NumericTraits<MeasurementType>::ZeroValue();
  MeasurementType busyness    = NumericTraits<MeasurementType>::ZeroValue();
  MeasurementType complexity  = NumericTraits<MeasurementType>::ZeroValue();
  MeasurementType strength    = NumericTraits<MeasurementType>::ZeroValue();

  typedef typename HistogramType::ConstIterator HistogramIterator;

  long unsigned int Ng = inputHistogram->GetSize(0);
  std::cout << "No of bins: " << Ng << "  total number of valid voxels: " << this->m_TotalNumberOfValidVoxels << std::endl;

  double sumPiSi    = 0.0;
  double sumSi      = 0.0;
  unsigned int Ngp  = 0;
  unsigned int Nv   = 0;

  //Calculate sum(pi*si).
  for ( HistogramIterator hit = inputHistogram->Begin();
        hit != inputHistogram->End(); ++hit )
  {
    IndexType index = hit.GetIndex();

    MeasurementType ni = hit.GetFrequency();
    double si = m_siMatrix[index[0]];
    if ( ni == 0 )
    {
      continue;
    }
    sumPiSi += ni*si;
    sumSi += si;
    Ngp++;
    Nv += ni;
  }

  sumPiSi /= (Nv > 0) ? Nv : 1;


  //Calculate the other features
  //pi = ni / Nv, so we can simply calculate with ni instead of pi

  double pipj   = 0.0;
  double ipijpj = 0.0;
  double complexitySum = 0.0;
  double strengthSum = 0.0;

  for ( HistogramIterator hit = inputHistogram->Begin();
        hit != inputHistogram->End(); ++hit )
  {
    MeasurementVectorType measurement = hit.GetMeasurementVector();
    IndexType index = hit.GetIndex();
    //MITK_WARN << "current index " << index;

    MeasurementType ni = hit.GetFrequency();
    double si = m_siMatrix[index[0]];

    double i = measurement[0];

    for ( HistogramIterator hit2 = inputHistogram->Begin();
          hit2 != inputHistogram->End(); ++hit2 )
    {
      MeasurementVectorType measurement2 = hit2.GetMeasurementVector();
      IndexType index2 = hit2.GetIndex();

      MeasurementType nj= hit2.GetFrequency();
      double sj = m_siMatrix[index2[0]];

      double j = measurement2[0];

      pipj += ni*nj*(i-j)*(i-j);
      ipijpj += std::abs(i*ni - j*nj);

      if(ni != 0 && nj != 0)
        complexitySum += std::abs(i-j) * (ni*si + nj*sj)/(ni+nj);

      strengthSum += (ni+nj)*(i-j)*(i-j);
    }
  }


  //Calculate Coarseness
  coarseness = (sumPiSi == 0) ? 1e6 : 1.0 / sumPiSi;
  contrast = (Ngp <= 1 || Nv == 0) ? 0 : (1.0/(Ngp * (Ngp - 1))*pipj / Nv / Nv) * (sumSi / Nv / Nv);
  busyness = (Ngp <= 1 || ipijpj == 0) ? 0 : sumPiSi / ipijpj / Nv;
  complexity = (Nv == 0) ? 0: complexitySum / Nv / Nv;
  strength = (sumSi == 0 || Nv == 0) ? 0 : strengthSum / Nv / sumSi;

  //Calculate the other features.

  MeasurementObjectType* CoarsenessOutputObject =
      static_cast<MeasurementObjectType*>( this->ProcessObject::GetOutput( 0 ) );
  CoarsenessOutputObject->Set( coarseness );

  MeasurementObjectType* ContrastOutputObject =
      static_cast<MeasurementObjectType*>( this->ProcessObject::GetOutput( 1 ) );
  ContrastOutputObject->Set( contrast );

  MeasurementObjectType* BusynessOutputObject =
      static_cast<MeasurementObjectType*>( this->ProcessObject::GetOutput( 2 ) );
  BusynessOutputObject->Set( busyness );

  MeasurementObjectType* ComplexityOutputObject =
      static_cast<MeasurementObjectType*>( this->ProcessObject::GetOutput( 3 ) );
  ComplexityOutputObject->Set( complexity );

  MeasurementObjectType* StrengthOutputObject =
      static_cast<MeasurementObjectType*>( this->ProcessObject::GetOutput( 4 ) );
  StrengthOutputObject->Set( strength );

}

template<typename THistogram>
const
typename EnhancedHistogramToNeighbourhoodGreyLevelDifferenceFeaturesFilter<THistogram>::MeasurementObjectType*
EnhancedHistogramToNeighbourhoodGreyLevelDifferenceFeaturesFilter<THistogram>
::GetCoarsenessOutput() const
{
  return itkDynamicCastInDebugMode<const MeasurementObjectType*>(this->ProcessObject::GetOutput( 0 ) );
}

template<typename THistogram>
const
typename EnhancedHistogramToNeighbourhoodGreyLevelDifferenceFeaturesFilter<THistogram>::MeasurementObjectType*
EnhancedHistogramToNeighbourhoodGreyLevelDifferenceFeaturesFilter<THistogram>
::GetContrastOutput() const
{
  return itkDynamicCastInDebugMode<const MeasurementObjectType*>( this->ProcessObject::GetOutput( 1 ) );
}

template<typename THistogram>
const
typename EnhancedHistogramToNeighbourhoodGreyLevelDifferenceFeaturesFilter<THistogram>::MeasurementObjectType*
EnhancedHistogramToNeighbourhoodGreyLevelDifferenceFeaturesFilter<THistogram>
::GetBusynessOutput() const
{
  return itkDynamicCastInDebugMode<const MeasurementObjectType*>(this->ProcessObject::GetOutput( 2 ) );
}

template<typename THistogram>
const
typename EnhancedHistogramToNeighbourhoodGreyLevelDifferenceFeaturesFilter<THistogram>::MeasurementObjectType*
EnhancedHistogramToNeighbourhoodGreyLevelDifferenceFeaturesFilter<THistogram>
::GetComplexityOutput() const
{
  return itkDynamicCastInDebugMode<const MeasurementObjectType*>(this->ProcessObject::GetOutput( 3 ) );
}

template<typename THistogram>
const
typename EnhancedHistogramToNeighbourhoodGreyLevelDifferenceFeaturesFilter<THistogram>::MeasurementObjectType*
EnhancedHistogramToNeighbourhoodGreyLevelDifferenceFeaturesFilter<THistogram>
::GetStrengthOutput() const
{
  return itkDynamicCastInDebugMode<const MeasurementObjectType*>( this->ProcessObject::GetOutput( 4 ) );
}


template<typename THistogram>
typename EnhancedHistogramToNeighbourhoodGreyLevelDifferenceFeaturesFilter<THistogram>::MeasurementType
EnhancedHistogramToNeighbourhoodGreyLevelDifferenceFeaturesFilter<THistogram>
::GetCoarseness() const
{
  return this->GetCoarsenessOutput()->Get();
}

template<typename THistogram>
typename EnhancedHistogramToNeighbourhoodGreyLevelDifferenceFeaturesFilter<THistogram>::MeasurementType
EnhancedHistogramToNeighbourhoodGreyLevelDifferenceFeaturesFilter<THistogram>
::GetContrast() const
{
  return this->GetContrastOutput()->Get();
}

template<typename THistogram>
typename EnhancedHistogramToNeighbourhoodGreyLevelDifferenceFeaturesFilter<THistogram>::MeasurementType
EnhancedHistogramToNeighbourhoodGreyLevelDifferenceFeaturesFilter<THistogram>
::GetBusyness() const
{
  return this->GetBusynessOutput()->Get();
}

template<typename THistogram>
typename EnhancedHistogramToNeighbourhoodGreyLevelDifferenceFeaturesFilter<THistogram>::MeasurementType
EnhancedHistogramToNeighbourhoodGreyLevelDifferenceFeaturesFilter<THistogram>
::GetComplexity() const
{
  return this->GetComplexityOutput()->Get();
}

template<typename THistogram>
typename EnhancedHistogramToNeighbourhoodGreyLevelDifferenceFeaturesFilter<THistogram>::MeasurementType
EnhancedHistogramToNeighbourhoodGreyLevelDifferenceFeaturesFilter<THistogram>
::GetStrength() const
{
  return this->GetStrengthOutput()->Get();
}


template<typename THistogram>
typename EnhancedHistogramToNeighbourhoodGreyLevelDifferenceFeaturesFilter< THistogram>::MeasurementType
EnhancedHistogramToNeighbourhoodGreyLevelDifferenceFeaturesFilter<THistogram>
::GetFeature( NeighbourhoodGreyLevelDifferenceFeatureName feature )
{
  switch( feature )
  {
  case Coarseness:
    return this->GetCoarseness();
  case Contrast:
    return this->GetContrast();
  case Busyness:
    return this->GetBusyness();
  case Complexity:
    return this->GetComplexity();
  case Strength:
    return this->GetStrength();
  default:
    return 0;
  }
}

template< typename THistogram>
void
EnhancedHistogramToNeighbourhoodGreyLevelDifferenceFeaturesFilter< THistogram>::
PrintSelf(std::ostream& os, Indent indent) const
{
  Superclass::PrintSelf( os,indent );
}
} // end of namespace Statistics
} // end of namespace itk

#endif
