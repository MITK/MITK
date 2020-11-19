/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef _mitkExtendedLabelStatisticsImageFilter_hxx
#define _mitkExtendedLabelStatisticsImageFilter_hxx

#include "mitkExtendedLabelStatisticsImageFilter.h"

#include "itkImageRegionConstIteratorWithIndex.h"
#include "itkImageRegionConstIterator.h"
#include <mbilog.h>
#include <mitkLogMacros.h>
#include "mitkNumericConstants.h"
#include "mitkLogMacros.h"
#include <mitkHistogramStatisticsCalculator.h>

namespace itk
{
  template< class TInputImage, class TLabelImage >
  bool
    ExtendedLabelStatisticsImageFilter< TInputImage, TLabelImage >
    ::GetMaskingNonEmpty() const
  {
    return m_MaskNonEmpty;
  }

  template< typename TInputImage, typename TLabelImage >
  void
  ExtendedLabelStatisticsImageFilter< TInputImage, TLabelImage >
  ::SetHistogramParameters(const int numBins, RealType lowerBound, RealType upperBound)
  {
    m_NumBins[0] = numBins;
    m_LowerBound = lowerBound;
    m_UpperBound = upperBound;
    m_GlobalHistogramParametersSet = true;
    m_PreferGlobalHistogramParameters = true;
    this->Modified();
  }

  template< typename TInputImage, typename TLabelImage >
  void
  ExtendedLabelStatisticsImageFilter< TInputImage, TLabelImage >
  ::SetHistogramParametersForLabels(std::map<LabelPixelType, unsigned int> numBins, std::map<LabelPixelType, PixelType> lowerBound,
                                       std::map<LabelPixelType, PixelType> upperBound)
  {
    m_LabelMin = lowerBound;
    m_LabelMax = upperBound;
    m_LabelNBins = numBins;
    m_LabelHistogramParametersSet = true;
    m_PreferGlobalHistogramParameters = false;
    this->Modified();
  }

  template< class TInputImage, class TLabelImage >
  typename ExtendedLabelStatisticsImageFilter< TInputImage, TLabelImage >::RealType
    ExtendedLabelStatisticsImageFilter< TInputImage, TLabelImage >
    ::GetUniformity(LabelPixelType label) const
  {
    StatisticsMapConstIterator      mapIt;

    mapIt = m_LabelStatistics.find(label);
    if ( mapIt == m_LabelStatistics.end() )
    {
      mitkThrow() << "Label does not exist";
    }
    else
    {
      return ( *mapIt ).second.m_Uniformity;
    }
  }


  template< class TInputImage, class TLabelImage >
  typename ExtendedLabelStatisticsImageFilter< TInputImage, TLabelImage >::RealType
    ExtendedLabelStatisticsImageFilter< TInputImage, TLabelImage >
    ::GetMedian(LabelPixelType label) const
  {
    StatisticsMapConstIterator      mapIt;

    mapIt = m_LabelStatistics.find(label);
    if ( mapIt == m_LabelStatistics.end() )
    {
      mitkThrow() << "Label does not exist";
    }
    else
    {
      return ( *mapIt ).second.m_Median;
    }
  }

  template< class TInputImage, class TLabelImage >
  typename ExtendedLabelStatisticsImageFilter< TInputImage, TLabelImage >::RealType
    ExtendedLabelStatisticsImageFilter< TInputImage, TLabelImage >
    ::GetEntropy(LabelPixelType label) const
  {
    StatisticsMapConstIterator      mapIt;

    mapIt = m_LabelStatistics.find(label);
    if ( mapIt == m_LabelStatistics.end() )
    {
      mitkThrow() << "Label does not exist";
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
    StatisticsMapConstIterator      mapIt;

    mapIt = m_LabelStatistics.find(label);
    if ( mapIt == m_LabelStatistics.end() )
    {
      mitkThrow() << "Label does not exist";
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
    StatisticsMapConstIterator      mapIt;

    mapIt = m_LabelStatistics.find(label);
    if ( mapIt == m_LabelStatistics.end() )
    {
      mitkThrow() << "Label does not exist";
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
    StatisticsMapConstIterator      mapIt;

    mapIt = m_LabelStatistics.find(label);
    if ( mapIt == m_LabelStatistics.end() )
    {
      mitkThrow() << "Label does not exist";
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
    StatisticsMapConstIterator      mapIt;

    mapIt = m_LabelStatistics.find(label);
    if ( mapIt == m_LabelStatistics.end() )
    {
      mitkThrow() << "Label does not exist";
    }
    else
    {
      return ( *mapIt ).second.m_Skewness;
    }
  }



  template< typename TInputImage, typename TLabelImage >
  typename ExtendedLabelStatisticsImageFilter< TInputImage, TLabelImage >::RealType
  ExtendedLabelStatisticsImageFilter< TInputImage, TLabelImage >
  ::GetMinimum(LabelPixelType label) const
  {
    StatisticsMapConstIterator mapIt;

    mapIt = m_LabelStatistics.find(label);
    if ( mapIt == m_LabelStatistics.end() )
      {
      mitkThrow() << "Label does not exist";
      }
    else
      {
      return ( *mapIt ).second.m_Minimum;
      }
  }

  template< typename TInputImage, typename TLabelImage >
  typename ExtendedLabelStatisticsImageFilter< TInputImage, TLabelImage >::RealType
  ExtendedLabelStatisticsImageFilter< TInputImage, TLabelImage >
  ::GetMaximum(LabelPixelType label) const
  {
    StatisticsMapConstIterator mapIt;

    mapIt = m_LabelStatistics.find(label);
    if ( mapIt == m_LabelStatistics.end() )
      {
      mitkThrow() << "Label does not exist";
      }
    else
      {
      return ( *mapIt ).second.m_Maximum;
      }
  }

  template< typename TInputImage, typename TLabelImage >
  typename ExtendedLabelStatisticsImageFilter< TInputImage, TLabelImage >::RealType
  ExtendedLabelStatisticsImageFilter< TInputImage, TLabelImage >
  ::GetMean(LabelPixelType label) const
  {
    StatisticsMapConstIterator mapIt;

    mapIt = m_LabelStatistics.find(label);
    if ( mapIt == m_LabelStatistics.end() )
      {
      mitkThrow() << "Label does not exist";
      }
    else
      {
      return ( *mapIt ).second.m_Mean;
      }
  }

  template< typename TInputImage, typename TLabelImage >
  typename ExtendedLabelStatisticsImageFilter< TInputImage, TLabelImage >::RealType
  ExtendedLabelStatisticsImageFilter< TInputImage, TLabelImage >
  ::GetSum(LabelPixelType label) const
  {
    StatisticsMapConstIterator mapIt;

    mapIt = m_LabelStatistics.find(label);
    if ( mapIt == m_LabelStatistics.end() )
      {
      mitkThrow() << "Label does not exist";
      }
    else
      {
      return ( *mapIt ).second.m_Sum;
      }
  }

  template< typename TInputImage, typename TLabelImage >
  typename ExtendedLabelStatisticsImageFilter< TInputImage, TLabelImage >::RealType
  ExtendedLabelStatisticsImageFilter< TInputImage, TLabelImage >
  ::GetSigma(LabelPixelType label) const
  {
    StatisticsMapConstIterator mapIt;

    mapIt = m_LabelStatistics.find(label);
    if ( mapIt == m_LabelStatistics.end() )
      {
      mitkThrow() << "Label does not exist";
      }
    else
      {
      return ( *mapIt ).second.m_Sigma;
      }
  }

  template< typename TInputImage, typename TLabelImage >
  typename ExtendedLabelStatisticsImageFilter< TInputImage, TLabelImage >::RealType
  ExtendedLabelStatisticsImageFilter< TInputImage, TLabelImage >
  ::GetVariance(LabelPixelType label) const
  {
    StatisticsMapConstIterator mapIt;

    mapIt = m_LabelStatistics.find(label);
    if ( mapIt == m_LabelStatistics.end() )
      {
      mitkThrow() << "Label does not exist";
      }
    else
      {
      return ( *mapIt ).second.m_Variance;
      }
  }

  template< typename TInputImage, typename TLabelImage >
  typename ExtendedLabelStatisticsImageFilter< TInputImage, TLabelImage >::BoundingBoxType
  ExtendedLabelStatisticsImageFilter< TInputImage, TLabelImage >
  ::GetBoundingBox(LabelPixelType label) const
  {
    StatisticsMapConstIterator mapIt;

    mapIt = m_LabelStatistics.find(label);
    if ( mapIt == m_LabelStatistics.end() )
      {
      mitkThrow() << "Label does not exist";
      }
    else
      {
      return ( *mapIt ).second.m_BoundingBox;
      }
  }

  template< typename TInputImage, typename TLabelImage >
  typename ExtendedLabelStatisticsImageFilter< TInputImage, TLabelImage >::RegionType
  ExtendedLabelStatisticsImageFilter< TInputImage, TLabelImage >
  ::GetRegion(LabelPixelType label) const
  {
    StatisticsMapConstIterator mapIt;

    mapIt = m_LabelStatistics.find(label);

    if ( mapIt == m_LabelStatistics.end() )
      {
      mitkThrow() << "Label does not exist";
      }
    else
      {
      typename Superclass::BoundingBoxType bbox = this->GetBoundingBox(label);
      typename Superclass::IndexType       index;
      typename Superclass::SizeType        size;

      unsigned int dimension = bbox.size() / 2;

      for ( unsigned int i = 0; i < dimension; i++ )
        {
        index[i] = bbox[2 * i];
        size[i] = bbox[2 * i + 1] - bbox[2 * i] + 1;
        }
      typename Superclass::RegionType region;
      region.SetSize(size);
      region.SetIndex(index);

      return region;
      }
  }

  template< typename TInputImage, typename TLabelImage >
  typename ExtendedLabelStatisticsImageFilter< TInputImage, TLabelImage >::MapSizeType
  ExtendedLabelStatisticsImageFilter< TInputImage, TLabelImage >
  ::GetCount(LabelPixelType label) const
  {
    StatisticsMapConstIterator mapIt;

    mapIt = m_LabelStatistics.find(label);
    if ( mapIt == m_LabelStatistics.end() )
      {
      mitkThrow() << "Label does not exist";
      }
    else
      {
      return ( *mapIt ).second.m_Count;
      }
  }


  template< typename TInputImage, typename TLabelImage >
  typename ExtendedLabelStatisticsImageFilter< TInputImage, TLabelImage >::HistogramType::Pointer
  ExtendedLabelStatisticsImageFilter< TInputImage, TLabelImage >
  ::GetHistogram(LabelPixelType label) const
  {
    StatisticsMapConstIterator mapIt;

    mapIt = m_LabelStatistics.find(label);
    if ( mapIt == m_LabelStatistics.end() )
      {
      mitkThrow() << "Label does not exist";
      }
    else
      {
      // this will be zero if histograms have not been enabled
      return ( *mapIt ).second.m_Histogram;
      }
  }



  template< typename TInputImage, typename TLabelImage >
  void
  ExtendedLabelStatisticsImageFilter< TInputImage, TLabelImage >
  ::BeforeThreadedGenerateData()
  {
    ThreadIdType numberOfThreads = this->GetNumberOfThreads();

    // Resize the thread temporaries
    m_LabelStatisticsPerThread.resize(numberOfThreads);

    // Initialize the temporaries
    for ( ThreadIdType i = 0; i < numberOfThreads; ++i )
      {
      m_LabelStatisticsPerThread[i].clear();
      }

    // Initialize the final map
    m_LabelStatistics.clear();
  }

  template< typename TInputImage, typename TLabelImage >
  std::list<int>
    ExtendedLabelStatisticsImageFilter< TInputImage, TLabelImage >
    ::GetRelevantLabels() const
  {
    std::list< int> relevantLabels;
    for (int i = 0; i < 4096; ++i )
    {
      if ( this->HasLabel( i ) )
      {
        relevantLabels.push_back( i );
      }
    }
    return relevantLabels;
  }

  template< typename TInputImage, typename TLabelImage >
  void
  ExtendedLabelStatisticsImageFilter< TInputImage, TLabelImage >
  ::ThreadedGenerateData(const typename TInputImage::RegionType & outputRegionForThread,
                         ThreadIdType threadId)
  {

    typename HistogramType::IndexType histogramIndex(1);
    typename HistogramType::MeasurementVectorType histogramMeasurement(1);

    const SizeValueType size0 = outputRegionForThread.GetSize(0);
    if( size0 == 0)
      {
      return;
      }

    ImageLinearConstIteratorWithIndex< TInputImage > it (this->GetInput(),
                                                         outputRegionForThread);

    ImageScanlineConstIterator< TLabelImage > labelIt (this->GetLabelInput(),
                                                       outputRegionForThread);

    StatisticsMapIterator mapIt;

    // support progress methods/callbacks
    const size_t numberOfLinesToProcess = outputRegionForThread.GetNumberOfPixels() / size0;
    ProgressReporter progress( this, threadId, numberOfLinesToProcess );

    typedef typename MapType::value_type MapValueType;

    // do the work
    while ( !it.IsAtEnd() )
      {
      while ( !it.IsAtEndOfLine() )
        {
        const RealType & value = static_cast< RealType >( it.Get() );

        const LabelPixelType & label = labelIt.Get();

        // is the label already in this thread?
        mapIt = m_LabelStatisticsPerThread[threadId].find(label);
        if ( mapIt == m_LabelStatisticsPerThread[threadId].end() )
          {
          // if global histogram parameters are set and preferred then use them
          if ( m_PreferGlobalHistogramParameters && m_GlobalHistogramParametersSet )
            {
            mapIt = m_LabelStatisticsPerThread[threadId].insert( MapValueType( label,
                                                                               LabelStatistics(m_NumBins[0], m_LowerBound,
                                                                                               m_UpperBound) ) ).first;
            }
          // if we have label histogram parameters then use them. If we encounter a label that has no parameters then use global settings if available
          else if(!m_PreferGlobalHistogramParameters && m_LabelHistogramParametersSet)
          {
            typename std::map<LabelPixelType, PixelType>::iterator lbIt, ubIt;
            typename std::map<LabelPixelType, unsigned int>::iterator nbIt;

            lbIt = m_LabelMin.find(label);
            ubIt = m_LabelMax.find(label);
            nbIt = m_LabelNBins.find(label);

            // if any of the parameters is lacking for the current label but global histogram params are available, use the global parameters
            if ((lbIt == m_LabelMin.end() || ubIt == m_LabelMax.end() || nbIt == m_LabelNBins.end()) && m_GlobalHistogramParametersSet)
            {
              mapIt = m_LabelStatisticsPerThread[threadId].insert( MapValueType( label,
                                                                                 LabelStatistics(m_NumBins[0], m_LowerBound,
                                                                                                 m_UpperBound) ) ).first;
            }
            // if any of the parameters is lacking for the current label and global histogram params are not available, dont use histograms for this label
            else if ((lbIt == m_LabelMin.end() || ubIt == m_LabelMax.end() || nbIt == m_LabelNBins.end()) && !m_GlobalHistogramParametersSet)
            {
              mapIt = m_LabelStatisticsPerThread[threadId].insert( MapValueType( label,
                                                                                 LabelStatistics() ) ).first;
            }
            // label histogram parameters are available, use them!
            else
            {
              PixelType lowerBound, upperBound;
              unsigned int nBins;
              lowerBound = (*lbIt).second;
              upperBound = (*ubIt).second;
              nBins = (*nbIt).second;
              mapIt = m_LabelStatisticsPerThread[threadId].insert( MapValueType( label,
                                                                                 LabelStatistics(nBins, lowerBound, upperBound) ) ).first;
            }
          }
          // neither global nor label specific histogram parameters are set -> don't use histograms
          else
            {
            mapIt = m_LabelStatisticsPerThread[threadId].insert( MapValueType( label,
                                                                               LabelStatistics() ) ).first;
            }
          }

        typename MapType::mapped_type &labelStats = ( *mapIt ).second;

        // update the values for this label and this thread
        if ( value < labelStats.m_Minimum )
          {
          labelStats.m_Minimum = value;
          }
        if ( value > labelStats.m_Maximum )
          {
          labelStats.m_Maximum = value;
          }

        // bounding box is min,max pairs
        for ( unsigned int i = 0; i < ( 2 * TInputImage::ImageDimension ); i += 2 )
          {
          const typename TInputImage::IndexType & index = it.GetIndex();
          if ( labelStats.m_BoundingBox[i] > index[i / 2] )
            {
            labelStats.m_BoundingBox[i] = index[i / 2];
            }
          if ( labelStats.m_BoundingBox[i + 1] < index[i / 2] )
            {
            labelStats.m_BoundingBox[i + 1] = index[i / 2];
            }
          }

        labelStats.m_Sum += value;
        labelStats.m_SumOfSquares += ( value * value );
        labelStats.m_Count++;
        labelStats.m_SumOfCubes += std::pow(value, 3.);
        labelStats.m_SumOfQuadruples += std::pow(value, 4.);

        if (value > 0)
        {
          labelStats.m_PositivePixelCount++;
          labelStats.m_SumOfPositivePixels += value;
        }

        // if enabled, update the histogram for this label
        if ( labelStats.m_Histogram.IsNotNull() )
        {
          histogramMeasurement[0] = value;
          labelStats.m_Histogram->GetIndex(histogramMeasurement, histogramIndex);
          labelStats.m_Histogram->IncreaseFrequencyOfIndex(histogramIndex, 1);
        }

        ++labelIt;
        ++it;
        }
      labelIt.NextLine();
      it.NextLine();
      progress.CompletedPixel();
      }

  }


  template< class TInputImage, class TLabelImage >
  void ExtendedLabelStatisticsImageFilter< TInputImage, TLabelImage >::
    AfterThreadedGenerateData()
  {
    StatisticsMapIterator      mapIt;
    StatisticsMapConstIterator threadIt;
    ThreadIdType               i;
    ThreadIdType     numberOfThreads = this->GetNumberOfThreads();

    // Run through the map for each thread and accumulate the count,
    // sum, and sumofsquares
    for ( i = 0; i < numberOfThreads; i++ )
      {
      // iterate over the map for this thread
      for ( threadIt = m_LabelStatisticsPerThread[i].begin();
            threadIt != m_LabelStatisticsPerThread[i].end();
            ++threadIt )
        {
        // does this label exist in the cumulative structure yet?
        mapIt = m_LabelStatistics.find( ( *threadIt ).first );
        if ( mapIt == m_LabelStatistics.end() )
          {
          // create a new entry
          typedef typename MapType::value_type MapValueType;
          if ( m_GlobalHistogramParametersSet || m_LabelHistogramParametersSet )
            {
//            mapIt = m_LabelStatistics.insert( MapValueType( ( *threadIt ).first,
//                                                            LabelStatistics(m_NumBins[0], m_LowerBound,
//                                                                            m_UpperBound) ) ).first;
            mapIt = m_LabelStatistics.insert( MapValueType( *threadIt ) ).first;
            continue;
            }
          else
            {
            mapIt = m_LabelStatistics.insert( MapValueType( ( *threadIt ).first,
                                                            LabelStatistics() ) ).first;
            }
          }


        typename MapType::mapped_type &labelStats = ( *mapIt ).second;

        // accumulate the information from this thread
        labelStats.m_Count += ( *threadIt ).second.m_Count;
        labelStats.m_Sum += ( *threadIt ).second.m_Sum;
        labelStats.m_SumOfSquares += ( *threadIt ).second.m_SumOfSquares;
        labelStats.m_SumOfPositivePixels += ( *threadIt ).second.m_SumOfPositivePixels;
        labelStats.m_PositivePixelCount +=  ( *threadIt ).second.m_PositivePixelCount;
        labelStats.m_SumOfCubes +=  ( *threadIt ).second.m_SumOfCubes;
        labelStats.m_SumOfQuadruples +=  ( *threadIt ).second.m_SumOfQuadruples;

        if ( labelStats.m_Minimum > ( *threadIt ).second.m_Minimum )
          {
          labelStats.m_Minimum = ( *threadIt ).second.m_Minimum;
          }
        if ( labelStats.m_Maximum < ( *threadIt ).second.m_Maximum )
          {
          labelStats.m_Maximum = ( *threadIt ).second.m_Maximum;
          }

        //bounding box is min,max pairs
        int dimension = labelStats.m_BoundingBox.size() / 2;
        for ( int ii = 0; ii < ( dimension * 2 ); ii += 2 )
          {
          if ( labelStats.m_BoundingBox[ii] > ( *threadIt ).second.m_BoundingBox[ii] )
            {
            labelStats.m_BoundingBox[ii] = ( *threadIt ).second.m_BoundingBox[ii];
            }
          if ( labelStats.m_BoundingBox[ii + 1] < ( *threadIt ).second.m_BoundingBox[ii + 1] )
            {
            labelStats.m_BoundingBox[ii + 1] = ( *threadIt ).second.m_BoundingBox[ii + 1];
            }
          }

        // if enabled, update the histogram for this label
        if ( m_GlobalHistogramParametersSet || m_LabelHistogramParametersSet )
          {
          typename HistogramType::IndexType index;
          index.SetSize(1);
          for ( unsigned int bin = 0; bin < labelStats.m_Histogram->Size(); bin++ )
            {
            index[0] = bin;
            labelStats.m_Histogram->IncreaseFrequency( bin, ( *threadIt ).second.m_Histogram->GetFrequency(bin) );
            }
          }
        } // end of thread map iterator loop
      }   // end of thread loop

    // compute the remainder of the statistics
    for ( mapIt = m_LabelStatistics.begin();
          mapIt != m_LabelStatistics.end();
          ++mapIt )
      {
      typename MapType::mapped_type &labelStats = ( *mapIt ).second;

      // mean
      labelStats.m_Mean = labelStats.m_Sum
                                 / static_cast< RealType >( labelStats.m_Count );

      // MPP
      labelStats.m_MPP = labelStats.m_SumOfPositivePixels
                          / static_cast< RealType >( labelStats.m_PositivePixelCount );

      // variance
      if ( labelStats.m_Count > 0 )
        {
        // unbiased estimate of variance
        LabelStatistics & ls = mapIt->second;
        const RealType    sumSquared  = ls.m_Sum * ls.m_Sum;
        const RealType    count       = static_cast< RealType >( ls.m_Count );

        ls.m_Variance = ( ls.m_SumOfSquares - sumSquared / count ) / ( count );

        RealType secondMoment = ls.m_SumOfSquares / count;
        RealType thirdMoment = ls.m_SumOfCubes / count;
        RealType fourthMoment = ls.m_SumOfQuadruples / count;

        ls.m_Skewness = (thirdMoment - 3. * secondMoment * ls.m_Mean + 2. * std::pow(ls.m_Mean, 3.)) / std::pow(secondMoment - std::pow(ls.m_Mean, 2.), 1.5); // see http://www.boost.org/doc/libs/1_51_0/doc/html/boost/accumulators/impl/skewness_impl.html
        ls.m_Kurtosis = (fourthMoment - 4. * thirdMoment * ls.m_Mean + 6. * secondMoment * std::pow(ls.m_Mean, 2.) - 3. * std::pow(ls.m_Mean, 4.)) / std::pow(secondMoment - std::pow(ls.m_Mean, 2.), 2.); // see http://www.boost.org/doc/libs/1_51_0/doc/html/boost/accumulators/impl/kurtosis_impl.html, dropped -3
        }
      else
        {
        labelStats.m_Variance = NumericTraits< RealType >::ZeroValue();
        labelStats.m_Skewness = NumericTraits< RealType >::ZeroValue();
        labelStats.m_Kurtosis = NumericTraits< RealType >::ZeroValue();
        }

      // sigma
      labelStats.m_Sigma = std::sqrt( labelStats.m_Variance );

      // histogram statistics
      if (labelStats.m_Histogram.IsNotNull())
      {
        mitk::HistogramStatisticsCalculator histStatCalc;
        histStatCalc.SetHistogram(labelStats.m_Histogram);
        histStatCalc.CalculateStatistics();
        labelStats.m_Median = histStatCalc.GetMedian();
        labelStats.m_Entropy = histStatCalc.GetEntropy();
        labelStats.m_Uniformity = histStatCalc.GetUniformity();
        labelStats.m_UPP = histStatCalc.GetUPP();
      }

      }

      {
      //Now update the cached vector of valid labels.
      m_ValidLabelValues.resize(0);
      m_ValidLabelValues.reserve(m_LabelStatistics.size());
      for ( mapIt = m_LabelStatistics.begin();
        mapIt != m_LabelStatistics.end();
        ++mapIt )
        {
        m_ValidLabelValues.push_back(mapIt->first);
        }
      }
  }

} // end namespace itk

#endif
