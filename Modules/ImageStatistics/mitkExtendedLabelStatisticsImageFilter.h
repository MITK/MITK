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
  * \brief Extension of the itkLabelStatisticsImageFilter that also calculates the Skewness,Kurtosis,Entropy,Uniformity.
  *
  * This class inherits from the itkLabelStatisticsImageFilter and
  * uses its results for the calculation of six additional coefficients:
  * the Skewness,Kurtosis,Uniformity,UPP,MPP,Entropy
  *
  * As these coefficient are based on the mean and the sigma which are both calculated
  * by the LabelStatisticsImageFilter, the method AfterThreadedGenerateData() is overwritten
  * and calls ComputeSkewnessKurtosisAndMPP() and ComputeEntropyUniformityAndUPP after the AfterThreadedGenerateData()
  * while the second coefficient Method is only called when the the method useHistogram is on!!!
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
    typedef  itk::Statistics::Histogram<double> HistogramType;

    itkFactorylessNewMacro( Self );
    itkCloneMacro( Self );
    itkTypeMacro(ExtendedLabelStatisticsImageFilter, LabelStatisticsImageFilter);

    /**
    * \brief Internal class which stores the calculated coefficients Skewness,Kurtosis,Uniformity,UPP,MPP,Entropy.
    */
    class CoefficientsClass
    {
    public:

      CoefficientsClass()
      {
        m_Kurtosis = 0.0;
        m_Skewness = 0.0;
        m_Entropy = -1.0;
        m_Uniformity = 0.0;
        m_MPP = 0.0;
        m_UPP = 0.0;
      };

      ~CoefficientsClass(){};

      /* the new member coefficients*/
      RealType m_Kurtosis;
      RealType m_Skewness;
      RealType m_Entropy;
      RealType m_Uniformity;
      RealType m_MPP;
      RealType m_UPP;
    };


    /*getter method for the new coefficients*/
    RealType GetSkewness(LabelPixelType label) const;
    RealType GetKurtosis(LabelPixelType label) const;
    RealType GetUniformity( LabelPixelType label) const;
    RealType GetEntropy( LabelPixelType label) const;
    RealType GetMPP( LabelPixelType label) const;
    RealType GetUPP( LabelPixelType label) const;

    std::list< int>  GetRelevantLabels() const;
    bool             GetMaskingNonEmpty() const;

  protected:

    typedef std::map< LabelPixelType, CoefficientsClass >        CoefficientsMap;
    typedef typename CoefficientsMap::const_iterator             CoefficientsMapConstIterator;

    ExtendedLabelStatisticsImageFilter();

    virtual ~ExtendedLabelStatisticsImageFilter(){};

    /**
    * \brief  ComputeSkewnessKurtosisAndMPP(),ComputeEntropyUniformityAndUPP() will be called after superclass
    * both methods are seprated because one is build up on the pixel values and one is build up a step after on a
    * histogram
    */
    void ComputeSkewnessKurtosisAndMPP();
    void AfterThreadedGenerateData();
    void ComputeEntropyUniformityAndUPP();
    void CalculateSettingsForLabels();

  private:

    CoefficientsMap         m_LabelStatisticsCoefficients;
    std::list< int>         m_RelevantLabels;
    bool                    m_MaskNonEmpty;

  }; // end of class

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "mitkExtendedLabelStatisticsImageFilter.hxx"
#endif

#endif
