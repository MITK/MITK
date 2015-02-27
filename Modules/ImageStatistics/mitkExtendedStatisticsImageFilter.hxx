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

    ComputeSkewnessAndKurtosis();
  }

  template< class TInputImage >
  void
    ExtendedStatisticsImageFilter< TInputImage >
    ::ComputeSkewnessAndKurtosis()
  {
    RealType mean = this->GetMean();
    RealType sigma = this->GetSigma();
    RealType baseOfSkewnessAndKurtosis;
    RealType kurtosis(0.0);
    RealType skewness(0.0);

    if (  sigma == 0 )
    {
      throw std::logic_error( "Empty segmentation" );
    }


    ImageRegionConstIterator< TInputImage > it (this->GetInput(), this->GetInput()->GetLargestPossibleRegion() );

    int counter = 0;
    for (it.GoToBegin(); !it.IsAtEnd(); ++it)
    {
      baseOfSkewnessAndKurtosis = (it.Get() - mean) / sigma ;
      kurtosis += std::pow( baseOfSkewnessAndKurtosis, 4.0 );
      skewness += std::pow( baseOfSkewnessAndKurtosis, 3.0 );
      counter++;
    }

    if ( counter == 0 )
    {
      throw std::logic_error( "Empty segmentation" );
    }

    kurtosis = kurtosis / counter;
    skewness = skewness / counter;


    this->GetKurtosisOutput()->Set( kurtosis );
    this->GetSkewnessOutput()->Set( skewness );
  }
}

#endif
