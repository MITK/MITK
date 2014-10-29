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
#ifndef __mitkStatisticsImageFilter_hxx
#define __mitkStatisticsImageFilter_hxx

#include "mitkStatisticsImageFilter.h"

namespace itk
{
template< class TInputImage >
ExtendedStatisticsImageFilter< TInputImage >::ExtendedStatisticsImageFilter()
: StatisticsImageFilter()
{
  // add comment
  for ( int i = 7; i < 9; ++i )
  {
    typename RealObjectType::Pointer output =
      static_cast< RealObjectType * >( this->MakeOutput(i).GetPointer() );
    this->ProcessObject::SetNthOutput( i, output.GetPointer() );
  }

  this->GetSkewnessOutput()->Set( NumericTraits< RealType >::max() );
  this->GetKurtosisOutput()->Set( NumericTraits< RealType >::max() );

}

template< class TInputImage >
DataObject::Pointer
ExtendedStatisticsImageFilter< TInputImage >::MakeOutput( ProcessObject::DataObjectPointerArraySizeType output)
{
  switch ( output )
  {
    case 7:
    case 8:
    {
      return RealObjectType::New().GetPointer();
      break;
    }
    default:
    {
      // might as well make an image
      return Superclass::MakeOutput( output );
      break;
    }
  }
}


template< class TInputImage >
typename ExtendedStatisticsImageFilter< TInputImage >::RealObjectType *
ExtendedStatisticsImageFilter< TInputImage >
::GetSkewnessOutput()
{
  return static_cast< RealObjectType * >( this->ProcessObject::GetOutput(7) );
}

template< class TInputImage >
const typename ExtendedStatisticsImageFilter< TInputImage >::RealObjectType *
ExtendedStatisticsImageFilter< TInputImage >
::GetSkewnessOutput() const
{
  return static_cast< const RealObjectType * >( this->ProcessObject::GetOutput(7) );
}


template< class TInputImage >
typename ExtendedStatisticsImageFilter< TInputImage >::RealObjectType *
ExtendedStatisticsImageFilter< TInputImage >
::GetKurtosisOutput()
{
  return static_cast< RealObjectType * >( this->ProcessObject::GetOutput(8) );
}

template< class TInputImage >
const typename ExtendedStatisticsImageFilter< TInputImage >::RealObjectType *
ExtendedStatisticsImageFilter< TInputImage >
::GetKurtosisOutput() const
{
  return static_cast< const RealObjectType * >( this->ProcessObject::GetOutput(8) );
}



template< class TInputImage >
void
ExtendedStatisticsImageFilter< TInputImage >
::AfterThreadedGenerateData()
{
  Superclass::AfterThreadedGenerateData();

  computeTheSkewnessAndCurtosis();
}

template< class TInputImage >
void
ExtendedStatisticsImageFilter< TInputImage >
::ComputeTheSkewnessAndCurtosis()
{
  RealType mean = GetMean();
  RealType sigma = GetSigma();
  RealType baseOfSkewnessAndCurtosis;
  RealType Kurtosis = 0;
  RealType Skewness = 0;

  ImageRegionConstIterator< TInputImage > it (this->GetInput(), this->GetInput()->GetLargestPossibleRegion ()  );

  int counter = 0;
  for (it.GoToBegin(); !it.IsAtEnd(); ++it)
  {
   baseOfSkewnessAndCurtosis = (it.Get() - mean) / sigma ;
   Kurtosis += baseOfSkewnessAndCurtosis * baseOfSkewnessAndCurtosis * baseOfSkewnessAndCurtosis * baseOfSkewnessAndCurtosis;
   Skewness += baseOfSkewnessAndCurtosis * baseOfSkewnessAndCurtosis * baseOfSkewnessAndCurtosis;
   counter++;
  }

  Kurtosis = Kurtosis / counter;
  Skewness = Skewness / counter;

  this->GetKurtosisOutput()->Set( Kurtosis );
  this->GetSkewnessOutput()->Set( Skewness );
  }
}

#endif