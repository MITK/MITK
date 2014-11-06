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
#ifndef __mitkExtendedLabelStatisticsImageFilter_h
#define __mitkExtendedLabelStatisticsImageFilter_h

#include "itkLabelStatisticsImageFilter.h"
#include "itkSimpleDataObjectDecorator.h"
#include "itksys/hash_map.hxx"
#include "itkHistogram.h"
#include "itkFastMutexLock.h"
#include <vector>
#include "itkDataObject.h"

#include "mitkCommon.h"

namespace itk
{
  /**
  * \class ExtendedLabelStatisticsImageFilter
  * \brief Extension of the itkLabelStatisticsImageFilter that also calculates the Skewness and Kurtosis.
  *
  * This class inherits from the itkLabelStatisticsImageFilter and
  * uses its results for the calculation of two new coefficients of the statistics:
  * the Skewness and Kurtosis. Both will be added in this new class. This class can be
  * used for calculating the statistics for a multilabelImage
  */
  template< class TInputImage, class TLabelImage >
  class ExtendedLabelStatisticsImageFilter : public LabelStatisticsImageFilter< TInputImage,  TLabelImage >
  {
  public:

    typedef ExtendedLabelStatisticsImageFilter                      Self;
    typedef LabelStatisticsImageFilter < TInputImage, TLabelImage > Superclass;
    typedef SmartPointer< Self >                                    Pointer;
    typedef SmartPointer< const Self >                              ConstPointer;

    itkNewMacro(Self);

    itkTypeMacro(ExtendedLabelStatisticsImageFilter, LabelStatisticsImageFilter);

    /**
    * \brief Internal class which stores the calculated coefficients Skewness and Kurtosis.
    */
    class CoefficientsClass
    {
    public:

      CoefficientsClass()
      {
        m_Kurtosis = 0;
        m_Skewness = 0;
      };

      ~CoefficientsClass(){};

      /* the new member coefficients*/
      double m_Kurtosis;
      double m_Skewness;

    };

    typedef std::map< LabelPixelType, CoefficientsClass >        CoefficientsMap;
    typedef typename std::map< LabelPixelType, CoefficientsClass >::const_iterator CoefficientsMapConstIterator;

    /*getter method for the new coefficients*/
    RealType GetSkewness(LabelPixelType label) const;
    RealType GetKurtosis(LabelPixelType label) const;

  protected:

    ExtendedLabelStatisticsImageFilter();

    virtual ~ExtendedLabelStatisticsImageFilter(){};

    void AfterThreadedGenerateData();

    /**
    * \brief Calculate Skewness and Kurtosis.
    *
    * This method will calculate the new coefficients with sigma and mean value of the threaded generate data of the base class.
    */
    void ComputeTheSkewnessAndKurtosis();


  private:

    CoefficientsMap         m_LabelStatisticsCoefficients;


  }; // end of class

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "mitkExtendedLabelStatisticsImageFilter.hxx"
#endif

#endif
