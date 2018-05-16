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
#ifndef __mitkRandomImageSampler_h
#define __mitkRandomImageSampler_h

#include "MitkCLUtilitiesExports.h"

//MITK
#include <mitkImage.h>
#include "mitkImageToImageFilter.h"
#include <itkImage.h>

namespace mitk
{
  enum RandomImageSamplerMode
  {
    SINGLE_ACCEPTANCE_RATE,
    CLASS_DEPENDEND_ACCEPTANCE_RATE,
    SINGLE_NUMBER_OF_ACCEPTANCE,
    CLASS_DEPENDEND_NUMBER_OF_ACCEPTANCE
  };


  class MITKCLUTILITIES_EXPORT RandomImageSampler : public ImageToImageFilter
  {
  public:

    mitkClassMacro( RandomImageSampler , ImageToImageFilter );
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    itkSetMacro(SamplingMode, RandomImageSamplerMode);
    itkGetConstMacro(SamplingMode, RandomImageSamplerMode);

    itkSetMacro(AcceptanceRate, double);
    itkGetConstMacro(AcceptanceRate, double);

    //itkSetMacro(AcceptanceRateVector, std::vector<double>);
    void SetAcceptanceRateVector(std::vector<double> arg)
    {
      m_AcceptanceRateVector = arg;
    }

    itkGetConstMacro(AcceptanceRateVector, std::vector<double>);

    itkSetMacro(NumberOfSamples, unsigned int);
    itkGetConstMacro(NumberOfSamples, unsigned int);

    //itkSetMacro(NumberOfSamplesVector, std::vector<unsigned int>);
    void SetNumberOfSamplesVector(std::vector<unsigned int> arg)
    {
      m_NumberOfSamplesVector = arg;
    }

    itkGetConstMacro(NumberOfSamplesVector, std::vector<unsigned int>);

  private:
    /*!
    \brief standard constructor
    */
    RandomImageSampler();
    /*!
    \brief standard destructor
    */
    ~RandomImageSampler();
    /*!
    \brief Method generating the output information of this filter (e.g. image dimension, image type, etc.).
    The interface ImageToImageFilter requires this implementation. Everything is taken from the input image.
    */
    virtual void GenerateOutputInformation() override;
    /*!
    \brief Method generating the output of this filter. Called in the updated process of the pipeline.
    This method generates the smoothed output image.
    */
    virtual void GenerateData() override;

    /*!
    \brief Internal templated method calling the ITK bilteral filter. Here the actual filtering is performed.
    */
    template<typename TPixel, unsigned int VImageDimension>
    void ItkImageProcessing(const itk::Image<TPixel, VImageDimension>* itkImage);

    /*!
    \brief Internal templated method calling the ITK bilteral filter. Here the actual filtering is performed.
    */
    template<typename TPixel, unsigned int VImageDimension>
    void ItkImageProcessingClassDependendSampling(const itk::Image<TPixel, VImageDimension>* itkImage);

    /*!
    \brief Internal templated method calling the ITK bilteral filter. Here the actual filtering is performed.
    */
    template<typename TPixel, unsigned int VImageDimension>
    void ItkImageProcessingFixedNumberSampling(const itk::Image<TPixel, VImageDimension>* itkImage);

    /*!
    \brief Internal templated method calling the ITK bilteral filter. Here the actual filtering is performed.
    */
    template<typename TPixel, unsigned int VImageDimension>
    void ItkImageProcessingClassDependendNumberSampling(const itk::Image<TPixel, VImageDimension>* itkImage);

    double m_AcceptanceRate;
    std::vector<double> m_AcceptanceRateVector;
    unsigned int m_NumberOfSamples;
    std::vector<unsigned int> m_NumberOfSamplesVector;
    RandomImageSamplerMode m_SamplingMode;
  };
} //END mitk namespace
#endif
