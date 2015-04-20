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
#ifndef __mitkExtendedStatisticsImageFilter_hxx
#define __mitkExtendedStatisticsImageFilter_hxx

#include "mitkExtendedStatisticsImageFilter.h"


namespace itk
{
  template< class TInputImage >
  ExtendedStatisticsImageFilter< TInputImage >::ExtendedStatisticsImageFilter()
    : StatisticsImageFilter< TInputImage >()
  {
    /*
    * add the Skewness and Kurtosis to the other statistical calculated Values
    * of the mitkStatisticsImageFilter as the 7th and 8th Output
    */
    for ( int i = 7; i < 13; ++i )
    {
      typename RealObjectType::Pointer output =
        static_cast< RealObjectType * >( this->MakeOutput(i).GetPointer() );
      this->ProcessObject::SetNthOutput( i, output.GetPointer() );
    }

    this->GetSkewnessOutput()->Set( NumericTraits< RealType >::max() );
    this->GetKurtosisOutput()->Set( NumericTraits< RealType >::max() );
    this->GetEntropyOutput()->Set( NumericTraits< RealType >::max() );
    this->GetUniformityOutput()->Set( NumericTraits< RealType >::max() );
    this->GetUPPOutput()->Set( NumericTraits< RealType >::max() );
    this->GetMPPOutput()->Set( NumericTraits< RealType >::max() );

    this->m_histogramCalculated = false;
    this->m_histogramGenerator =  HistogramGeneratorType::New();
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
  void  ExtendedStatisticsImageFilter< TInputImage >
    ::SetBinSize( int size )
  {
    m_Binsize = size;
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
  void
    ExtendedStatisticsImageFilter< TInputImage >
    ::AfterThreadedGenerateData()
  {
    Superclass::AfterThreadedGenerateData();

    ComputeSkewnessKurtosisAndMPP();

    CalculteHistogram();

    if(m_histogramCalculated == true)
    {
      ComputeEntropyUniformityAndUPP();
    }
    else
    {
      MITK_WARN << "Cannot compute coefficients UPP,Entropy,Uniformity because of missing histogram";
    }
  }

  template< class TInputImage >
  void
    ExtendedStatisticsImageFilter< TInputImage >
    ::ComputeSkewnessKurtosisAndMPP()
  {
    RealType mean = this->GetMean();
    RealType sigma = this->GetSigma();
    RealType baseOfSkewnessAndKurtosis;
    RealType kurtosis(0.0);
    RealType skewness(0.0);
    RealType mpp(0.0);
    RealType currentPixel(0.0);

    if (  sigma == 0 )
    {
      throw std::logic_error( "Empty segmentation" );
    }


    ImageRegionConstIterator< TInputImage > it (this->GetInput(), this->GetInput()->GetLargestPossibleRegion() );

    int counter = 0;
    for (it.GoToBegin(); !it.IsAtEnd(); ++it)
    {
      currentPixel = it.Get();

      baseOfSkewnessAndKurtosis = (currentPixel - mean) / sigma ;
      kurtosis += std::pow( baseOfSkewnessAndKurtosis, 4.0 );
      skewness += std::pow( baseOfSkewnessAndKurtosis, 3.0 );

      if(currentPixel > 0)
      {
        mpp+= currentPixel;
      }
      counter++;
    }

    if ( counter == 0 )
    {
      throw std::logic_error( "Empty segmentation" );
    }

    kurtosis = kurtosis / counter;
    skewness = skewness / counter;
    mpp = mpp/counter;

    this->GetKurtosisOutput()->Set( kurtosis );
    this->GetSkewnessOutput()->Set( skewness );
    this->GetMPPOutput()->Set( mpp );
  }


  template< class TInputImage >
  void
    ExtendedStatisticsImageFilter< TInputImage >
    ::CalculteHistogram()
  {
    // Calculate histogram
       typedef itk::Statistics::ScalarImageToHistogramGenerator< TInputImage >
    HistogramGeneratorType;


    m_histogramGenerator->SetInput( this->GetInput() );
    m_histogramGenerator->SetMarginalScale( 100 );
    m_histogramGenerator->SetNumberOfBins( m_Binsize );
    m_histogramGenerator->SetHistogramMin( this->GetMinimum() );
    m_histogramGenerator->SetHistogramMax( this->GetMaximum() );
    m_histogramGenerator->Compute();

    //SetHistogramOutput(m_histogramGenerator->GetOutput() );

    m_histogramCalculated = true;

  }


  template< class TInputImage >
  void
    ExtendedStatisticsImageFilter< TInputImage >
    ::ComputeEntropyUniformityAndUPP()
  {
    double baseChange = std::log10(2);
    RealType partialProbability( 0.0 );
    RealType uniformity( 0.0 );
    RealType entropy( 0.0 );
    RealType upp( 0.0 );

        typename  const HistogramGeneratorType::HistogramType* histogramForEntropy = GetHistogram();

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
        this->GetEntropyOutput()->Set( entropy );
        this->GetUniformityOutput()->Set( uniformity );
        this->GetUPPOutput()->Set( upp );

  }
}

#endif
