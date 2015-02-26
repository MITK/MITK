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

#include "itkDataObject.h"

#include "MitkImageStatisticsExports.h"

#include "mitkCommon.h"

namespace itk
{
  /**
  * \class ExtendedStatisticsImageFilter
  * \brief Extension of the itkStatisticsImageFilter that also calculates the Skewness and Kurtosis.
  *
  * This class inherits from the itkStatisticsImageFilter and
  * uses its Results for the calculation of the two new coefficients:
  * the Skewness and Kurtosis. Both will be added in this class. So they will be
  * placed as the 7th and 8th Output after the other statistical coefficients
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
    typedef typename Superclass::RealType                     RealType;
    typedef typename Superclass::RealObjectType               RealObjectType;

    /** Method for creation through the object factory. */
    itkFactorylessNewMacro( Self );
    itkCloneMacro( Self );

    /** Runtime information support. */
    itkTypeMacro(ExtendedStatisticsImageFilter, StatisticsImageFilter);

    /**
    * \brief Return the computed Skewness.
    */
    RealType GetSkewness() const
    {
      return this->GetSkewnessOutput()->Get();
    }

    /**
    * \brief Return the computed Kurtosis.
    */
    RealType GetKurtosis() const
    {
      return this->GetKurtosisOutput()->Get();
    }

  protected:

    ExtendedStatisticsImageFilter();

    virtual ~ExtendedStatisticsImageFilter(){};

    void AfterThreadedGenerateData();

    /**
    * \brief Compute the Skewness Kurtosis.
    *
    * The Skewness and Kurtosis will be calculated with the Sigma and Mean Value of the
    * itkStatisticsImageFilter which comes out of the threadedGenerateData().
    */
    void ComputeTheSkewnessAndKurtosis();


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
