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

namespace itk
{
  /**
  * \class ExtendedLabelStatisticsImageFilter
  * \brief Extension of the itkLabelStatisticsImageFilter that also calculates the Skewness and Kurtosis.
  *
  * This class inherits from the itkLabelStatisticsImageFilter and
  * uses its results for the calculation of two additional coefficients:
  * the Skewness and Kurtosis.
  *
  * As these coefficient are based on the mean and the sigma which are both calculated
  * by the LabelStatisticsImageFilter, the method AfterThreadedGenerateData() is overwritten
  * and calls ComputeSkewnessAndKurtosis() after the AfterThreadedGenerateData()
  * implementation of the superclass is called.
  *
  */
  template< class TInputImage, class TLabelImage >
  class ExtendedLabelStatisticsImageFilter : public LabelStatisticsImageFilter< TInputImage,  TLabelImage >
  {
  public:

    typedef ExtendedLabelStatisticsImageFilter                      Self;
    typedef LabelStatisticsImageFilter < TInputImage, TLabelImage > Superclass;
    typedef SmartPointer< Self >                                    Pointer;
    typedef SmartPointer< const Self >                              ConstPointer;
    typedef typename Superclass::LabelPixelType                     LabelPixelType;
    typedef typename Superclass::RealType                           RealType;
    typedef typename Superclass::PixelType                          PixelType;
    typedef typename Superclass::MapIterator                        MapIterator;

    itkFactorylessNewMacro( Self );
    itkCloneMacro( Self );
    itkTypeMacro(ExtendedLabelStatisticsImageFilter, LabelStatisticsImageFilter);

    /**
    * \brief Internal class which stores the calculated coefficients Skewness and Kurtosis.
    */
    class CoefficientsClass
    {
    public:

      CoefficientsClass()
      {
        m_Kurtosis = 0.0;
        m_Skewness = 0.0;
      };

      ~CoefficientsClass(){};

      /* the new member coefficients*/
      RealType m_Kurtosis;
      RealType m_Skewness;

    };


    /*getter method for the new coefficients*/
    RealType GetSkewness(LabelPixelType label) const;
    RealType GetKurtosis(LabelPixelType label) const;

  protected:

    typedef std::map< LabelPixelType, CoefficientsClass >        CoefficientsMap;
    typedef typename CoefficientsMap::const_iterator             CoefficientsMapConstIterator;

    ExtendedLabelStatisticsImageFilter();

    virtual ~ExtendedLabelStatisticsImageFilter(){};

    /**
    * brief Calls AfterThreadedGenerateData() of the superclass and ComputeSkewnessAndKurtosis().
    */
    void AfterThreadedGenerateData();

    /**
    * \brief Calculate Skewness and Kurtosis.
    *
    * This method will calculate the new coefficients with sigma and mean value of the threaded generate data of the base class.
    */
    void ComputeSkewnessAndKurtosis();


  private:

    CoefficientsMap         m_LabelStatisticsCoefficients;


  }; // end of class

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "mitkExtendedLabelStatisticsImageFilter.hxx"
#endif

#endif
