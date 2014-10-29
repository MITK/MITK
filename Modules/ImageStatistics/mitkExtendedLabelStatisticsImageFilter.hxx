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
#ifndef _mitkExtendedLabelStatisticsImageFilter_h
#define _mitkExtendedLabelStatisticsImageFilter_h
#include "mitkExtendedLabelStatisticsImageFilter.h"

#include "itkImageRegionIterator.h"
#include "itkImageRegionConstIteratorWithIndex.h"
#include "itkProgressReporter.h"

#include "itkImageFileWriter.h"

namespace itk
{
  template< class TInputImage , class TLabelImage>
  ExtendedLabelStatisticsImageFilter< TInputImage, TLabelImage >
    ::ExtendedLabelStatisticsImageFilter()
    : LabelStatisticsImageFilter()
  {
    m_LabelStatisticsCoefficients;
  }


  template< class TInputImage, class TLabelImage >
  typename ExtendedLabelStatisticsImageFilter< TInputImage, TLabelImage >::RealType
    ExtendedLabelStatisticsImageFilter< TInputImage, TLabelImage >
    ::GetKurtosis(LabelPixelType label) const
  {
    CoefficientsMapConstIterator mapIt;

    mapIt = m_LabelStatisticsCoefficients.find(label);
    if ( mapIt == m_LabelStatisticsCoefficients.end() )
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
    CoefficientsMapConstIterator mapIt;

    mapIt = m_LabelStatisticsCoefficients.find(label);
    if ( mapIt == m_LabelStatisticsCoefficients.end() )
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
    ComputeTheSkewnessAndCurtosis()
  {
    MapIterator mapIt;
    TLabelImage::RegionType Subregion;

    double baseOfSkewnessAndCurtosis;
    double Kurtosis = 0.0;
    double Skewness = 0.0;

    std::list< LabelPixelType> relevantLabels;
    bool maskNonEmpty = false;
    LabelPixelType i;
    for ( i = 1; i < 100; ++i )
    {
      if ( this->HasLabel( i ) )
      {
        relevantLabels.push_back( i );
        maskNonEmpty = true;
        m_LabelStatisticsCoefficients.insert( std::make_pair(i, CoefficientsClass()) );
      }
    }

    if ( maskNonEmpty )
    {
      std::list< LabelPixelType >::iterator it;
      for ( it = relevantLabels.begin(), i = 0;
        it != relevantLabels.end();
        ++it, ++i )
      {
        double Sigma = GetSigma( *it );
        double Mean     = GetMean( *it );
        Subregion = Superclass::GetRegion(*it);

        ImageRegionConstIteratorWithIndex< TInputImage > it1 (this->GetInput(),
          Subregion);
        ImageRegionConstIterator< TLabelImage > labelIt (this->GetLabelInput(),
          Subregion);

        for (it1.GoToBegin(); !it1.IsAtEnd(); ++it1, ++labelIt)
        {
          if (labelIt.Get() == *it)
          {
            baseOfSkewnessAndCurtosis = (  it1.Get() -Mean ) / Sigma ;
            Kurtosis += baseOfSkewnessAndCurtosis*baseOfSkewnessAndCurtosis*baseOfSkewnessAndCurtosis*baseOfSkewnessAndCurtosis;
            Skewness += baseOfSkewnessAndCurtosis*baseOfSkewnessAndCurtosis*baseOfSkewnessAndCurtosis;
          }
        }

        m_LabelStatisticsCoefficients[*it].m_Skewness = double(Skewness/GetCount(*it));
        m_LabelStatisticsCoefficients[*it].m_Kurtosis = double(Kurtosis/GetCount(*it));
      }
    }
  }


  template< class TInputImage, class TLabelImage >
  void ExtendedLabelStatisticsImageFilter< TInputImage, TLabelImage >::
    AfterThreadedGenerateData()
  {
    Superclass::AfterThreadedGenerateData();

    ComputeTheSkewnessAndCurtosis();
  }

} // end namespace itk

#endif