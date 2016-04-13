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
#include <mbilog.h>
#include <mitkLogMacros.h>
#include "mitkNumericConstants.h"
#include "mitkLogMacros.h"

namespace itk
{
  template< class TInputImage , class TLabelImage>
  ExtendedLabelStatisticsImageFilter< TInputImage, TLabelImage >
    ::ExtendedLabelStatisticsImageFilter()
    : LabelStatisticsImageFilter< TInputImage,  TLabelImage >()
  {
    CalculateSettingsForLabels();
  }


  template< class TInputImage, class TLabelImage >
  std::list<int>
    ExtendedLabelStatisticsImageFilter< TInputImage, TLabelImage >
    ::GetRelevantLabels() const
  {
    return m_RelevantLabels;
  }


  template< class TInputImage, class TLabelImage >
  bool
    ExtendedLabelStatisticsImageFilter< TInputImage, TLabelImage >
    ::GetMaskingNonEmpty() const
  {
    return m_MaskNonEmpty;
  }


  template< class TInputImage, class TLabelImage >
  typename ExtendedLabelStatisticsImageFilter< TInputImage, TLabelImage >::RealType
    ExtendedLabelStatisticsImageFilter< TInputImage, TLabelImage >
    ::GetUniformity(LabelPixelType label) const
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
      return ( *mapIt ).second.m_Uniformity;
    }
  }

  template< class TInputImage, class TLabelImage >
  typename ExtendedLabelStatisticsImageFilter< TInputImage, TLabelImage >::RealType
    ExtendedLabelStatisticsImageFilter< TInputImage, TLabelImage >
    ::GetEntropy(LabelPixelType label) const
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
      return ( *mapIt ).second.m_Entropy;
    }
  }

  template< class TInputImage, class TLabelImage >
  typename ExtendedLabelStatisticsImageFilter< TInputImage, TLabelImage >::RealType
    ExtendedLabelStatisticsImageFilter< TInputImage, TLabelImage >
    ::GetUPP(LabelPixelType label) const
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
      return ( *mapIt ).second.m_UPP;
    }
  }

  template< class TInputImage, class TLabelImage >
  typename ExtendedLabelStatisticsImageFilter< TInputImage, TLabelImage >::RealType
    ExtendedLabelStatisticsImageFilter< TInputImage, TLabelImage >
    ::GetMPP(LabelPixelType label) const
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
      return ( *mapIt ).second.m_MPP;
    }
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
  void ExtendedLabelStatisticsImageFilter< TInputImage, TLabelImage >::
    CalculateSettingsForLabels()
  {
    LabelPixelType i;
    m_MaskNonEmpty = false;
    for ( i = 1; i < 4096; ++i )
    {
      if ( this->HasLabel( i ) )
      {
        m_RelevantLabels.push_back( i );
        m_MaskNonEmpty = true;
        m_LabelStatisticsCoefficients.insert( std::make_pair(i, CoefficientsClass()) );
      }
    }
  }


  template< class TInputImage, class TLabelImage >
  void
    ExtendedLabelStatisticsImageFilter< TInputImage, TLabelImage >::
    ComputeEntropyUniformityAndUPP()
  {
    double baseChange = std::log10(2);
    RealType partialProbability( 0.0 );
    RealType uniformity( 0.0 );
    RealType entropy( 0.0 );
    RealType upp( 0.0 );

    LabelPixelType i;
    if ( m_MaskNonEmpty )
    {
      typename std::list< int >::const_iterator it;
      for ( it = m_RelevantLabels.cbegin(), i = 0;
        it != m_RelevantLabels.cend();
        ++it, ++i )
      {
        HistogramType::Pointer histogramForEntropy = this->GetHistogram(*it);
        for (int i = 0; i < histogramForEntropy->Size(); i++)
        {
          partialProbability = histogramForEntropy->GetFrequency(i,0) / double ( histogramForEntropy->GetTotalFrequency() ) ;

          if( partialProbability != 0)
          {
            entropy -= partialProbability *( std::log10(partialProbability) / std::log10(2) ) ;
            uniformity += std::pow(partialProbability,2);


            if(histogramForEntropy->GetMeasurement(i,0) > 0)
            {
              upp += std::pow(partialProbability,2);
            }
          }
        }
        m_LabelStatisticsCoefficients[*it].m_Entropy = entropy;
        m_LabelStatisticsCoefficients[*it].m_Uniformity = uniformity;
        m_LabelStatisticsCoefficients[*it].m_UPP = upp;
      }
    }
  }

  template< class TInputImage, class TLabelImage >
  void
    ExtendedLabelStatisticsImageFilter< TInputImage, TLabelImage >::
    ComputeSkewnessKurtosisAndMPP()
  {
    typename TLabelImage::RegionType Subregion;

    RealType baseOfSkewnessAndCurtosis( 0.0 );
    RealType kurtosis( 0.0 );
    RealType skewness( 0.0 );
    RealType mpp( 0.0 );
    RealType currentPixel( 0.0 );

    std::list< LabelPixelType> relevantLabels;
    LabelPixelType i;
    if ( m_MaskNonEmpty )
    {
      typename std::list< int >::const_iterator it;
      for ( it = m_RelevantLabels.cbegin(), i = 0;
        it != m_RelevantLabels.cend();
        ++it )
      {
        RealType sigma = this->GetSigma( *it );
        RealType mean  = this->GetMean( *it );
        Subregion = Superclass::GetRegion(*it);

        int count( this->GetCount(*it) );
        if ( count == 0 || sigma < mitk::eps)
        {
          throw std::logic_error( "Empty segmentation" );
        }

        if ( fabs( sigma ) < mitk::sqrteps )
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
            currentPixel = it1.Get();
            baseOfSkewnessAndCurtosis = (currentPixel -mean) / sigma;
            kurtosis += std::pow( baseOfSkewnessAndCurtosis, 4.0 );
            skewness += std::pow( baseOfSkewnessAndCurtosis, 3.0 );

            if(currentPixel > 0)
            {
              mpp+= currentPixel;
            }

          }
        }
        m_LabelStatisticsCoefficients[*it].m_Skewness = RealType(skewness/count);
        m_LabelStatisticsCoefficients[*it].m_Kurtosis = RealType(kurtosis/count);
        m_LabelStatisticsCoefficients[*it].m_MPP = RealType(mpp/count);
      }
    }
  }



  template< class TInputImage, class TLabelImage >
  void ExtendedLabelStatisticsImageFilter< TInputImage, TLabelImage >::
    AfterThreadedGenerateData()
  {
    Superclass::AfterThreadedGenerateData();

    CalculateSettingsForLabels();

    ComputeSkewnessKurtosisAndMPP();

    if(this->GetUseHistograms())
    {
      ComputeEntropyUniformityAndUPP();
    }
    else
    {
      MITK_WARN << "Cannot compute coefficients UPP,Entropy,Uniformity because of missing histogram";
    }
  }

} // end namespace itk

#endif
