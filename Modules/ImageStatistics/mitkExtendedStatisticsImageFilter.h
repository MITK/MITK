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
#ifndef __mitkExtendedStatisticsImageFilter_h
#define __mitkExtendedStatisticsImageFilter_h

#include "itkStatisticsImageFilter.h"
#include "itkScalarImageToHistogramGenerator.h"
#include <mbilog.h>
#include <mitkLogMacros.h>

namespace itk
{
  /**
  * \class ExtendedStatisticsImageFilter
  * \brief Extension of the itkStatisticsImageFilter that also calculates the Skewness and Kurtosis.
  *
  * This class inherits from the itkStatisticsImageFilter and
  * uses its results for the calculation of other additional coefficients:
  * the Skewness and Kurtosis.
  *
  * As these coefficient are based on the mean and the sigma which are both calculated
  * by the StatisticsImageFilter, the method AfterThreadedGenerateData() is overwritten
  * and calls ComputeSkewnessKurtosisAndMPP() and ComputeEntropyUniformityAndUPP
  * after the AfterThreadedGenerateData()
  * implementation of the superclass is called.
  *
  * As the StatisticsImageFilter stores the statistics in the outputs 1 to 6 by the
  * StatisticsImageFilter, the skewness, kurtosis,MPP,UPP,Uniformity and Entropy are stored in the outputs
  * 7 to 13 by this filter.
  */
  template< class TInputImage >
  class ExtendedStatisticsImageFilter : public StatisticsImageFilter< TInputImage >
  {
  public:
    /** Standard Self typedef */
    typedef ExtendedStatisticsImageFilter                    Self;
    typedef StatisticsImageFilter< TInputImage >             Superclass;
    typedef SmartPointer< Self >                             Pointer;
    typedef SmartPointer< const Self >                       ConstPointer;
    typedef typename Superclass::RealType                    RealType;
    typedef typename Superclass::RealObjectType              RealObjectType;
    typedef typename Superclass::PixelType                   PixelType;

    itkFactorylessNewMacro( Self );
    itkCloneMacro( Self );
    itkTypeMacro( ExtendedStatisticsImageFilter, StatisticsImageFilter );


        typedef itk::Statistics::ScalarImageToHistogramGenerator< TInputImage >
          HistogramGeneratorType;

    /**
    * \brief Return the computed Skewness.
    */
    double GetSkewness() const
    {
      return this->GetSkewnessOutput()->Get();
    }

    /**
    * \brief Return the computed Median
    */
    double GetMedian() const
    {
        return this->GetMedianOutput()->Get();
    }

    /**
    * \brief Return the computed Kurtosis.
    */
    double GetKurtosis() const
    {
      return this->GetKurtosisOutput()->Get();
    }

    /* \brief Return the computed MPP.
      */
      double GetMPP() const
    {
      return this->GetMPPOutput()->Get();
    }

    /**
    * \brief Return the computed Uniformity.
    */
    double GetUniformity() const
    {
      return this->GetUniformityOutput()->Get();
    }

    /**
      *\brief Return the computed Entropy.
      */
      double GetEntropy() const
    {
      return this->GetEntropyOutput()->Get();
    }

    /**
      * \brief Return the computed UPP.
      */
      double GetUPP() const
    {
      return this->GetUPPOutput()->Get();
    }


      /**
      * \brief Return the computed Histogram.
      */
      const typename HistogramGeneratorType::HistogramType*
        GetHistogram()
      {
        return m_HistogramGenerator->GetOutput();
      }

      /**
      * \brief Set the Binsize for the Histogram.
      */
    void SetBinSize(int size);

  protected:

    ExtendedStatisticsImageFilter();

    virtual ~ExtendedStatisticsImageFilter(){};

    /**
    * brief Calls AfterThreadedGenerateData() of the superclass and the main methods
    */
    void AfterThreadedGenerateData();
    void CalculateHistogram();

    /**
    * \brief Compute Entropy,uniformity,MPP,UPP, Median.
    *
    * The Entropy,uniformity,MPP, Median and UPP will be calculated with the Sigma, Histogram and Mean Value of the
    * itkStatisticsImageFilter which comes out of the threadedGenerateData().
    */
    void ComputeSkewnessKurtosisAndMPP();
    void ComputeEntropyUniformityMedianAndUPP();

  /**
    * \brief Histogram.
    *
    * new members for setting and calculating the hisotgram for those coefficients which depends on this
    */
    typename  HistogramGeneratorType::Pointer  m_HistogramGenerator;
    int m_BinSize;
    bool m_HistogramCalculated;


    RealObjectType* GetSkewnessOutput();

    const RealObjectType* GetSkewnessOutput() const;

    RealObjectType* GetKurtosisOutput();

    const RealObjectType* GetKurtosisOutput() const;

    RealObjectType* GetMPPOutput();

    const RealObjectType* GetMPPOutput() const;

    RealObjectType* GetEntropyOutput();

    const RealObjectType* GetEntropyOutput() const;

    RealObjectType* GetUniformityOutput();

    const RealObjectType* GetUniformityOutput() const;

    RealObjectType* GetUPPOutput();

    const RealObjectType* GetUPPOutput() const;

    RealObjectType* GetMedianOutput();

    const RealObjectType* GetMedianOutput() const;

    virtual DataObject::Pointer MakeOutput( ProcessObject::DataObjectPointerArraySizeType idx );


  }; // end of class

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "mitkExtendedStatisticsImageFilter.hxx"
#endif

#endif
