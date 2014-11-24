*=========================================================================
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
#ifndef __itkLabelStatisticsImageFilter_hxx
#define __itkLabelStatisticsImageFilter_hxx
#include "itkLabelStatisticsImageFilter.h"

#include "itkImageRegionIterator.h"
#include "itkImageRegionConstIteratorWithIndex.h"
#include "itkProgressReporter.h"

namespace itk
{



template< class TInputImage, class TLabelImage >
typename ExtendedLabelStatisticsImageFilter< TInputImage, TLabelImage >::RealType
ExtendedLabelStatisticsImageFilter< TInputImage, TLabelImage >
::GetKurtosis(LabelPixelType label) const
{
  MapConstIterator mapIt;

  mapIt = m_LabelStatistics.find(label);
  if ( mapIt == m_LabelStatistics.end() )
    {
    // label does not exist, return a default value
    return NumericTraits< PixelType >::Zero;
    }
  else
    {
    return ( *mapIt ).second.m_Kurtosis;
    }
}


template< class TInputImage, class TLabelImage >
typename ExtendedLabelStatisticsImageFilter< TInputImage, TLabelImage >::RealType
ExtendedLabelStatisticsImageFilter< TInputImage, TLabelImage >
::GetSkewness(LabelPixelType label) const
{
  MapConstIterator mapIt;

  mapIt = m_LabelStatistics.find(label);
  if ( mapIt == m_LabelStatistics.end() )
    {
    // label does not exist, return a default value
    return NumericTraits< PixelType >::Zero;
    }
  else
    {
    return ( *mapIt ).second.m_Skewness;
    }
}




template< class TInputImage, class TLabelImage >
void
ExtendedLabelStatisticsImageFilter< TInputImage, TLabelImage >::
 computeTheSkewnessAndCurtosis()
{

   std::list< int > relevantLabels;
  bool maskNonEmpty = false;
  unsigned int i;
  for ( i = 1; i < 4096; ++i )
  {
    if ( labelStatisticsFilter->HasLabel( i ) )
    {
      relevantLabels.push_back( i );
      maskNonEmpty = true;
    }
  }

  if ( maskNonEmpty )
  {
      std::list< int >::iterator it;
    for ( it = relevantLabels.begin(), i = 0;
      it != relevantLabels.end();
      ++it, ++i )
    {

     // RealType Varinace = GetVariance( *it );
      //RealType Mean     = GetSigma( *it );

         m_LabelStatistics2;

    }
  }


}

} // end namespace itk
#endif