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
#ifndef _mitkExtendedLabelStatisticsImageFilter_hxx
#define _mitkExtendedLabelStatisticsImageFilter_hxx

#include "mitkExtendedLabelStatisticsImageFilter.h"

#include "itkImageRegionConstIteratorWithIndex.h"

namespace itk
{
  template< class TInputImage , class TLabelImage>
  ExtendedLabelStatisticsImageFilter< TInputImage, TLabelImage >
    ::ExtendedLabelStatisticsImageFilter()
    : LabelStatisticsImageFilter< TInputImage,  TLabelImage >()
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
    ComputeTheSkewnessAndKurtosis()
  {
    MapIterator mapIt;
    typename TLabelImage::RegionType Subregion;

    double baseOfSkewnessAndCurtosis = 0.0;
    double kurtosis = 0.0;
    double skewness = 0.0;

    std::list< LabelPixelType> relevantLabels;
    bool maskNonEmpty = false;
    LabelPixelType i;
    for ( i = 1; i < 4096; ++i )
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
      typename std::list< LabelPixelType >::iterator it;
      for ( it = relevantLabels.begin(), i = 0;
        it != relevantLabels.end();
        ++it, ++i )
      {
        double sigma = GetSigma( *it );
        double mean  = GetMean( *it );
        Subregion = Superclass::GetRegion(*it);

        int count( GetCount(*it) );
        if ( count == 0 || sigma==0)
        {
          throw std::logic_error( "Empty segmentation" );
        }


        ImageRegionConstIteratorWithIndex< TInputImage > it1 (this->GetInput(),
          Subregion);
        ImageRegionConstIterator< TLabelImage > labelIt (this->GetLabelInput(),
          Subregion);

        for (it1.GoToBegin(); !it1.IsAtEnd(); ++it1, ++labelIt)
        {
          if (labelIt.Get() == *it)
          {
            baseOfSkewnessAndCurtosis = (it1.Get() -mean) / sigma;
            kurtosis += std::pow( baseOfSkewnessAndCurtosis, 4.0 );
            skewness += std::pow( baseOfSkewnessAndCurtosis, 3.0 );
          }
        }

        m_LabelStatisticsCoefficients[*it].m_Skewness = double(skewness/count);
        m_LabelStatisticsCoefficients[*it].m_Kurtosis = double(kurtosis/count);
      }
    }
  }


  template< class TInputImage, class TLabelImage >
  void ExtendedLabelStatisticsImageFilter< TInputImage, TLabelImage >::
    AfterThreadedGenerateData()
  {

    Superclass::AfterThreadedGenerateData();

    ComputeTheSkewnessAndKurtosis();
  }

} // end namespace itk

#endif
