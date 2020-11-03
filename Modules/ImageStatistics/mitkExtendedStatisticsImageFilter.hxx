/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef __mitkExtendedStatisticsImageFilter_hxx
#define __mitkExtendedStatisticsImageFilter_hxx

#include "mitkExtendedStatisticsImageFilter.h"
#include <mitkHistogramStatisticsCalculator.h>


namespace itk
{
  template< class TInputImage >
  ExtendedStatisticsImageFilter< TInputImage >::ExtendedStatisticsImageFilter()
    : StatisticsImageFilter< TInputImage >(),
      m_ThreadSum(1),
      m_SumOfSquares(1),
      m_SumOfCubes(1),
      m_SumOfQuadruples(1),
      m_Count(1),
      m_PositivePixelCount(1),
      m_ThreadSumOfPositivePixels(1),
      m_ThreadMin(1),
      m_ThreadMax(1),
      m_UseHistogram(false),
      m_HistogramCalculated(false)
  {
    /*
    * add the Skewness,Kurtosis,Entropy,Uniformity,MPP, UPP, Median to the other statistical calculated Values
    * of the mitkStatisticsImageFilter as the 7th to the 13th Output
    */
    for ( int i = 7; i < 14; ++i )
    {
      typename RealObjectType::Pointer output =
        static_cast< RealObjectType * >( this->MakeOutput(i).GetPointer() );
      this->ProcessObject::SetNthOutput( i, output.GetPointer() );
    }

    this->GetSkewnessOutput()->Set( 0.0 );
    this->GetKurtosisOutput()->Set( 0.0 );
    this->GetEntropyOutput()->Set( -1.0 );
    this->GetUniformityOutput()->Set( 0.0 );
    this->GetUPPOutput()->Set( 0.0 );
    this->GetMPPOutput()->Set( 0.0 );
    this->GetMedianOutput()->Set( 0.0 );

    m_Histogram = ITK_NULLPTR;
    m_HistogramPerThread.resize(0);
  }

  template< class TInputImage >
  DataObject::Pointer
    ExtendedStatisticsImageFilter< TInputImage >::MakeOutput( ProcessObject::DataObjectPointerArraySizeType output)
  {
    switch ( output )
    {
    case 7:
    case 8:
    case 9:
    case 10:
    case 11:
    case 12:
      {
        return RealObjectType::New().GetPointer();
      }
    case 13:
    {
      return RealObjectType::New().GetPointer();
    }
    default:
      {
        // might as well make an image
        return Superclass::MakeOutput( output );
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
  typename ExtendedStatisticsImageFilter< TInputImage >::RealObjectType *
    ExtendedStatisticsImageFilter< TInputImage >
    ::GetUniformityOutput()
  {
    return static_cast< RealObjectType * >( this->ProcessObject::GetOutput(9) );
  }

  template< class TInputImage >
  const typename ExtendedStatisticsImageFilter< TInputImage >::RealObjectType *
    ExtendedStatisticsImageFilter< TInputImage >
    ::GetUniformityOutput() const
  {
    return static_cast< const RealObjectType * >( this->ProcessObject::GetOutput(9) );
  }


  template< class TInputImage >
  typename ExtendedStatisticsImageFilter< TInputImage >::RealObjectType *
    ExtendedStatisticsImageFilter< TInputImage >
    ::GetEntropyOutput()
  {
    return static_cast< RealObjectType * >( this->ProcessObject::GetOutput(10) );
  }

  template< class TInputImage >
  const typename ExtendedStatisticsImageFilter< TInputImage >::RealObjectType *
    ExtendedStatisticsImageFilter< TInputImage >
    ::GetEntropyOutput() const
  {
    return static_cast< const RealObjectType * >( this->ProcessObject::GetOutput(10) );
  }

  template< class TInputImage >
  typename ExtendedStatisticsImageFilter< TInputImage >::RealObjectType *
    ExtendedStatisticsImageFilter< TInputImage >
    ::GetUPPOutput()
  {
    return static_cast< RealObjectType * >( this->ProcessObject::GetOutput(11) );
  }

  template< class TInputImage >
  const typename ExtendedStatisticsImageFilter< TInputImage >::RealObjectType *
    ExtendedStatisticsImageFilter< TInputImage >
    ::GetUPPOutput() const
  {
    return static_cast< const RealObjectType * >( this->ProcessObject::GetOutput(11) );
  }

  template< class TInputImage >
  typename ExtendedStatisticsImageFilter< TInputImage >::RealObjectType *
    ExtendedStatisticsImageFilter< TInputImage >
    ::GetMPPOutput()
  {
    return static_cast< RealObjectType * >( this->ProcessObject::GetOutput(12) );
  }

  template< class TInputImage >
  const typename ExtendedStatisticsImageFilter< TInputImage >::RealObjectType *
    ExtendedStatisticsImageFilter< TInputImage >
    ::GetMPPOutput() const
  {
    return static_cast< const RealObjectType * >( this->ProcessObject::GetOutput(12) );
  }

  template< class TInputImage >
  typename ExtendedStatisticsImageFilter< TInputImage >::RealObjectType *
    ExtendedStatisticsImageFilter< TInputImage >
    ::GetMedianOutput()
  {
    return static_cast< RealObjectType * >( this->ProcessObject::GetOutput(13) );
  }

  template< class TInputImage >
  const typename ExtendedStatisticsImageFilter< TInputImage >::RealObjectType *
    ExtendedStatisticsImageFilter< TInputImage >
    ::GetMedianOutput() const
  {
    return static_cast< const RealObjectType * >( this->ProcessObject::GetOutput(13) );
  }


  template< typename TInputImage >
  void
  ExtendedStatisticsImageFilter< TInputImage >
  ::BeforeThreadedGenerateData()
  {
    ThreadIdType numberOfThreads = this->GetNumberOfThreads();

    if (m_UseHistogram)
    {
      // Histogram
      m_Histogram = HistogramType::New();
      typename HistogramType::SizeType hsize;
      typename HistogramType::MeasurementVectorType lb;
      typename HistogramType::MeasurementVectorType ub;
      hsize.SetSize(1);
      lb.SetSize(1);
      ub.SetSize(1);
      m_Histogram->SetMeasurementVectorSize(1);
      hsize[0] = m_NumBins;
      lb[0] = m_LowerBound;
      ub[0] = m_UpperBound;
      m_Histogram->Initialize(hsize, lb, ub);

      m_HistogramPerThread.resize(numberOfThreads);

      for (unsigned int i = 0; i < numberOfThreads; i++)
      {
        typename HistogramType::Pointer hist = HistogramType::New();
        typename HistogramType::SizeType hsize;
        typename HistogramType::MeasurementVectorType lb;
        typename HistogramType::MeasurementVectorType ub;
        hsize.SetSize(1);
        lb.SetSize(1);
        ub.SetSize(1);
        hist->SetMeasurementVectorSize(1);
        hsize[0] = m_NumBins;
        lb[0] = m_LowerBound;
        ub[0] = m_UpperBound;
        hist->Initialize(hsize, lb, ub);
        m_HistogramPerThread[i] = hist;
      }
    }

    // Resize the thread temporaries
    m_Count.SetSize(numberOfThreads);
    m_SumOfSquares.SetSize(numberOfThreads);
    m_SumOfCubes.SetSize(numberOfThreads);
    m_SumOfQuadruples.SetSize(numberOfThreads);
    m_ThreadSum.SetSize(numberOfThreads);
    m_ThreadMin.SetSize(numberOfThreads);
    m_ThreadMax.SetSize(numberOfThreads);
    m_PositivePixelCount.SetSize(numberOfThreads);
    m_ThreadSumOfPositivePixels.SetSize(numberOfThreads);

    // Initialize the temporaries
    m_Count.Fill(NumericTraits< SizeValueType >::Zero);
    m_ThreadSum.Fill(NumericTraits< RealType >::Zero);
    m_SumOfSquares.Fill(NumericTraits< RealType >::Zero);
    m_SumOfCubes.Fill(NumericTraits< RealType >::Zero);
    m_SumOfQuadruples.Fill(NumericTraits< RealType >::Zero);
    m_ThreadMin.Fill( NumericTraits< PixelType >::max() );
    m_ThreadMax.Fill( NumericTraits< PixelType >::NonpositiveMin() );
    m_PositivePixelCount.Fill(NumericTraits< SizeValueType >::Zero);
    m_ThreadSumOfPositivePixels.Fill(NumericTraits< SizeValueType >::Zero);
  }


  template< typename TInputImage >
  void
  ExtendedStatisticsImageFilter< TInputImage >
  ::ThreadedGenerateData(const typename StatisticsImageFilter<TInputImage>::RegionType &
                                   outputRegionForThread,
                                   ThreadIdType threadId)
  {
    const SizeValueType size0 = outputRegionForThread.GetSize(0);
    if( size0 == 0)
      {
      return;
      }
    RealType  realValue;
    PixelType value;

    typename HistogramType::IndexType histogramIndex(1);
    typename HistogramType::MeasurementVectorType histogramMeasurement(1);

    RealType sum = NumericTraits< RealType >::ZeroValue();
    RealType sumOfPositivePixels = NumericTraits< RealType >::ZeroValue();
    RealType sumOfSquares = NumericTraits< RealType >::ZeroValue();
    RealType sumOfCubes = NumericTraits< RealType >::ZeroValue();
    RealType sumOfQuadruples = NumericTraits< RealType >::ZeroValue();
    SizeValueType count = NumericTraits< SizeValueType >::ZeroValue();
    SizeValueType countOfPositivePixels = NumericTraits< SizeValueType >::ZeroValue();
    PixelType min = NumericTraits< PixelType >::max();
    PixelType max = NumericTraits< PixelType >::NonpositiveMin();

    ImageScanlineConstIterator< TInputImage > it (this->GetInput(),  outputRegionForThread);

    // support progress methods/callbacks
    const size_t numberOfLinesToProcess = outputRegionForThread.GetNumberOfPixels() / size0;
    ProgressReporter progress( this, threadId, numberOfLinesToProcess );

    // do the work
    while ( !it.IsAtEnd() )
      {
      while ( !it.IsAtEndOfLine() )
        {
        value = it.Get();
        realValue = static_cast< RealType >( value );

        if (m_UseHistogram)
        {
          histogramMeasurement[0] = value;
          m_HistogramPerThread[threadId]->GetIndex(histogramMeasurement, histogramIndex);
          m_HistogramPerThread[threadId]->IncreaseFrequencyOfIndex(histogramIndex, 1);
        }

        if ( value < min )
          {
          min = value;
          }
        if ( value > max )
          {
          max  = value;
          }
        if (value > 0)
        {
          sumOfPositivePixels += realValue;
          ++countOfPositivePixels;
        }

        sum += realValue;
        sumOfSquares += ( realValue * realValue );
        sumOfCubes += std::pow(realValue, 3.);
        sumOfQuadruples += std::pow(realValue, 4.);
        ++count;
        ++it;
        }
      it.NextLine();
      progress.CompletedPixel();
      }

    m_ThreadSum[threadId] = sum;
    m_SumOfSquares[threadId] = sumOfSquares;
    m_Count[threadId] = count;
    m_ThreadMin[threadId] = min;
    m_ThreadMax[threadId] = max;
    m_ThreadSumOfPositivePixels[threadId] = sumOfPositivePixels;
    m_PositivePixelCount[threadId] = countOfPositivePixels;
    m_SumOfCubes[threadId] = sumOfCubes;
    m_SumOfQuadruples[threadId] = sumOfQuadruples;
  }


  template< class TInputImage >
  void
    ExtendedStatisticsImageFilter< TInputImage >
    ::AfterThreadedGenerateData()
  {
    ThreadIdType    i;
    SizeValueType   count = 0;
    SizeValueType   countOfPositivePixels = 0;
    RealType        sumOfSquares = 0;
    RealType        sumOfCubes = 0;
    RealType        sumOfQuadruples = 0;
    RealType        sumOfPositivePixels = 0;

    ThreadIdType numberOfThreads = this->GetNumberOfThreads();

    PixelType minimum;
    PixelType maximum;
    RealType  mean;
    RealType  meanOfPositivePixels;
    RealType  sigma;
    RealType  variance;
    RealType  skewness;
    RealType  kurtosis;
    RealType  sum;

    sum = sumOfSquares = sumOfCubes = sumOfQuadruples = NumericTraits< RealType >::ZeroValue();
    count = countOfPositivePixels = 0;

    // Find the min/max over all threads and accumulate count, sum and
    // sum of squares
    minimum = NumericTraits< PixelType >::max();
    maximum = NumericTraits< PixelType >::NonpositiveMin();
    for ( i = 0; i < numberOfThreads; i++ )
      {
      count += m_Count[i];
      sum += m_ThreadSum[i];
      sumOfSquares += m_SumOfSquares[i];
      sumOfCubes += m_SumOfCubes[i];
      sumOfQuadruples += m_SumOfQuadruples[i];
      sumOfPositivePixels += m_ThreadSumOfPositivePixels[i];
      countOfPositivePixels += m_PositivePixelCount[i];

      if ( m_ThreadMin[i] < minimum )
        {
        minimum = m_ThreadMin[i];
        }
      if ( m_ThreadMax[i] > maximum )
        {
        maximum = m_ThreadMax[i];
        }
      // if enabled, update the histogram for this label
      if ( m_UseHistogram )
        {
        typename HistogramType::IndexType index;
        index.SetSize(1);
        for ( int bin = 0; bin < m_NumBins; ++bin )
          {
          index[0] = bin;
          m_Histogram->IncreaseFrequency( bin, m_HistogramPerThread[i]->GetFrequency(bin) );
          }
        }
      }
    // compute statistics
    mean = sum / static_cast< RealType >( count );
    meanOfPositivePixels = sumOfPositivePixels / static_cast< RealType >( countOfPositivePixels );

    // unbiased estimate
    variance = ( sumOfSquares - ( sum * sum / static_cast< RealType >( count ) ) )
               / ( static_cast< RealType >( count ) );
    RealType secondMoment, thirdMoment, fourthMoment;
    secondMoment = sumOfSquares / static_cast< RealType >( count );
    thirdMoment = sumOfCubes / static_cast< RealType >( count );
    fourthMoment = sumOfQuadruples / static_cast< RealType >( count );

    skewness = (thirdMoment - 3. * secondMoment * mean + 2. * std::pow(mean, 3.)) / std::pow(secondMoment - std::pow(mean, 2.), 1.5); // see http://www.boost.org/doc/libs/1_51_0/doc/html/boost/accumulators/impl/skewness_impl.html
    kurtosis = (fourthMoment - 4. * thirdMoment * mean + 6. * secondMoment * std::pow(mean, 2.) - 3. * std::pow(mean, 4.)) / std::pow(secondMoment - std::pow(mean, 2.), 2.); // see http://www.boost.org/doc/libs/1_46_1/doc/html/boost/accumulators/impl/kurtosis_impl.html, dropped -3
    sigma = std::sqrt(variance);

    // Set the outputs
    this->GetMinimumOutput()->Set(minimum);
    this->GetMaximumOutput()->Set(maximum);
    this->GetMeanOutput()->Set(mean);
    this->GetSigmaOutput()->Set(sigma);
    this->GetVarianceOutput()->Set(variance);
    this->GetSumOutput()->Set(sum);
    this->GetKurtosisOutput()->Set(kurtosis);
    this->GetSkewnessOutput()->Set(skewness);
    this->GetMPPOutput()->Set(meanOfPositivePixels);

    if (m_UseHistogram)
    {
      mitk::HistogramStatisticsCalculator histStatCalc;
      histStatCalc.SetHistogram(m_Histogram);
      histStatCalc.CalculateStatistics();
      this->GetUniformityOutput()->Set(histStatCalc.GetUniformity());
      this->GetUPPOutput()->Set(histStatCalc.GetUPP());
      this->GetEntropyOutput()->Set(histStatCalc.GetEntropy());
      this->GetMedianOutput()->Set(histStatCalc.GetMedian());

    }
    m_HistogramCalculated = true;
  }

  template< typename TInputImage >
  void
  ExtendedStatisticsImageFilter< TInputImage >
  ::SetHistogramParameters(const int numBins, RealType lowerBound, RealType upperBound)
  {
    m_NumBins = numBins;
    m_LowerBound = lowerBound;
    m_UpperBound = upperBound;
    m_UseHistogram = true;
  }

}

#endif
