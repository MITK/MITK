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

namespace itk
{
  /**
  * \class ExtendedStatisticsImageFilter
  * \brief Extension of the itkStatisticsImageFilter that also calculates the Skewness and Kurtosis.
  *
  * This class inherits from the itkStatisticsImageFilter and
  * uses its results for the calculation of the two additional coefficients:
  * the Skewness and Kurtosis.
  *
  * As these coefficient are based on the mean and the sigma which are both calculated
  * by the StatisticsImageFilter, the method AfterThreadedGenerateData() is overwritten
  * and calls ComputeSkewnessAndKurtosis() after the AfterThreadedGenerateData()
  * implementation of the superclass is called.
  *
  * As the StatisticsImageFilter stores the statistics in the outputs 1 to 6 by the
  * StatisticsImageFilter, the skewness and kurtosis are stored in the outputs
  * 7 and 8 by this filter.
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
    * \brief Return the computed Kurtosis.
    */
    double GetKurtosis() const
    {
      return this->GetKurtosisOutput()->Get();
    }

  protected:

    ExtendedStatisticsImageFilter();

    virtual ~ExtendedStatisticsImageFilter(){};

    /**
    * brief Calls AfterThreadedGenerateData() of the superclass and ComputeSkewnessAndKurtosis().
    */
    void AfterThreadedGenerateData();

    /**
    * \brief Compute the Skewness Kurtosis.
    *
    * The Skewness and Kurtosis will be calculated with the Sigma and Mean Value of the
    * itkStatisticsImageFilter which comes out of the threadedGenerateData().
    */
    void ComputeSkewnessAndKurtosis();


    RealObjectType* GetSkewnessOutput();

    const RealObjectType* GetSkewnessOutput() const;

    RealObjectType* GetKurtosisOutput();

    const RealObjectType* GetKurtosisOutput() const;

    virtual DataObject::Pointer MakeOutput( ProcessObject::DataObjectPointerArraySizeType idx );


  }; // end of class

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "mitkExtendedStatisticsImageFilter.hxx"
#endif

#endif
