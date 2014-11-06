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

#include "ImageStatisticsExports.h"

#include "mitkCommon.h"

namespace itk
{
  /**
  * \class ExtendedStatisticsImageFilter
  * \brief
  * this class inherits from the itkStatisticsImageFilter and
  * calculate with the Results of the Statistics two new coefficients:
  * the Skewness and Kurtosis. Both will be added in this new class
  */
  template< class TInputImage >
  class ImageStatistics_EXPORT ExtendedStatisticsImageFilter : public StatisticsImageFilter< TInputImage >
  {
  public:
    /** Standard Self typedef */
    typedef ExtendedStatisticsImageFilter                    Self;
    typedef StatisticsImageFilter< TInputImage >             Superclass;
    typedef SmartPointer< Self >                             Pointer;
    typedef SmartPointer< const Self >                       ConstPointer;

    /** Method for creation through the object factory. */
    itkNewMacro( Self );

    /** Runtime information support. */
    itkTypeMacro(ExtendedStatisticsImageFilter, StatisticsImageFilter);

    typedef typename Superclass::DataObjectPointer DataObjectPointer;

    /** Return the computed Skewness. */
    RealType GetSkewness() const
    {
      return this->GetSkewnessOutput()->Get();
    }
    RealObjectType* GetSkewnessOutput();

    const RealObjectType* GetSkewnessOutput() const;


    /*Return the computed Kurtosis. */
    RealType GetKurtosis() const
    {
      return this->GetKurtosisOutput()->Get();
    }

    RealObjectType* GetKurtosisOutput();

    const RealObjectType* GetKurtosisOutput() const;

    virtual DataObject::Pointer MakeOutput( ProcessObject::DataObjectPointerArraySizeType idx );

    /*brief
    * Compute the Skewness Kurtosis.
    * the Skewness and Kurtosis will be calculated with the Sigma and Mean Value of the
    * itkStatisticsImageFilter which comes out of the threaded Generate Data
    */
    void ComputeTheSkewnessAndCurtosis();

  protected:

    ExtendedStatisticsImageFilter();

    virtual ~ExtendedStatisticsImageFilter(){};

    void AfterThreadedGenerateData();


  }; // end of class

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "mitkStatisticsImageFilter.hxx"
#endif

#endif
