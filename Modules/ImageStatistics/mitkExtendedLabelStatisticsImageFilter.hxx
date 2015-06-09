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
#include "itkImageRegionConstIterator.h"

#include "mitkNumericConstants.h"
#include "mitkLogMacros.h"

namespace itk
{
  template< class TInputImage , class TLabelImage>
  ExtendedLabelStatisticsImageFilter< TInputImage, TLabelImage >
    ::ExtendedLabelStatisticsImageFilter()
    : LabelStatisticsImageFilter< TInputImage,  TLabelImage >()
  { }


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
    ComputeSkewnessAndKurtosis()
  {
    typename TLabelImage::RegionType Subregion;

    RealType baseOfSkewnessAndCurtosis( 0.0 );
    RealType kurtosis( 0.0 );
    RealType skewness( 0.0 );

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
      typename std::list< LabelPixelType >::const_iterator it;
      for ( it = relevantLabels.cbegin();
        it != relevantLabels.cend();
        ++it )
      {
        RealType sigma = GetSigma( *it );
        RealType mean  = GetMean( *it );
        Subregion = Superclass::GetRegion(*it);

        int count( GetCount(*it) );
        if ( count == 0 )
        {
          throw std::logic_error( "Empty segmentation" );
        }

        if ( fabs( sigma ) < typename mitk::sqrteps )
        {
          throw std::logic_error( "Sigma == 0" );
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

        m_LabelStatisticsCoefficients[*it].m_Skewness = RealType(skewness/count);
        m_LabelStatisticsCoefficients[*it].m_Kurtosis = RealType(kurtosis/count);
      }
    }
  }


  template< class TInputImage, class TLabelImage >
  void ExtendedLabelStatisticsImageFilter< TInputImage, TLabelImage >::
    AfterThreadedGenerateData()
  {

    Superclass::AfterThreadedGenerateData();

    try
    {
      ComputeSkewnessAndKurtosis();
    }
    catch ( const std::exception& e )
    {
      MITK_ERROR << "Caught an exception during calculation of skewness and kurtosis: " << e.what();
    }

  }

} // end namespace itk

#endif
