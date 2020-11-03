/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef __mitkExtendedStatisticsImageFilter
#define __mitkExtendedStatisticsImageFilter

#include "itkStatisticsImageFilter.h"
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
  * As the StatisticsImageFilter stores the statistics in the outputs 1 to 6 by the
  * StatisticsImageFilter, the skewness, kurtosis, MPP, UPP, Uniformity, Entropy and Median are stored in the outputs
  * 7 to 14 by this filter.
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

    /** Histogram-related typedefs */
    typedef itk::Statistics::Histogram< RealType > HistogramType;
    typedef typename HistogramType::Pointer        HistogramPointer;

    itkFactorylessNewMacro( Self );
    itkCloneMacro( Self );
    itkTypeMacro( ExtendedStatisticsImageFilter, StatisticsImageFilter );

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
      const typename HistogramType::Pointer
        GetHistogram()
      {
          if (m_HistogramCalculated)
          {
              return m_Histogram;
          }
          else
          {
              return nullptr;
          }
      }


    /** specify Histogram parameters  */
    void SetHistogramParameters(const int numBins, RealType lowerBound,
                                RealType upperBound);

  protected:

    ExtendedStatisticsImageFilter();

    ~ExtendedStatisticsImageFilter() override{};

    void BeforeThreadedGenerateData() override;

    /** Multi-thread version GenerateData. */
    void  ThreadedGenerateData(const typename StatisticsImageFilter<TInputImage>::RegionType &
                               outputRegionForThread,
                               ThreadIdType threadId) override;

    /**
    * brief Calls AfterThreadedGenerateData() of the superclass and the main methods
    */
    void AfterThreadedGenerateData() override;


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

    using Superclass::MakeOutput;
    DataObject::Pointer MakeOutput( ProcessObject::DataObjectPointerArraySizeType idx ) override;

private:
    Array< RealType >       m_ThreadSum;
    Array< RealType >       m_SumOfSquares;
    Array< RealType >       m_SumOfCubes;
    Array< RealType >       m_SumOfQuadruples;
    Array< SizeValueType >  m_Count;
    Array< SizeValueType >  m_PositivePixelCount;
    Array< RealType >       m_ThreadSumOfPositivePixels;
    Array< PixelType >      m_ThreadMin;
    Array< PixelType >      m_ThreadMax;
    std::vector< HistogramPointer > m_HistogramPerThread;
    HistogramPointer        m_Histogram;
    bool                    m_UseHistogram;
    bool                    m_HistogramCalculated;
    RealType                m_LowerBound, m_UpperBound;
    int                     m_NumBins;


  }; // end of class

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "mitkExtendedStatisticsImageFilter.hxx"
#endif

#endif
