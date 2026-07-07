/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef mitkRandomImageSampler_h
#define mitkRandomImageSampler_h

#include <MitkCLUtilitiesExports.h>

//MITK
#include <mitkImage.h>
#include <mitkImageToImageFilter.h>
#include <itkImage.h>

namespace mitk
{
  /** \brief Enumeration of sampling modes for RandomImageSampler. */
  enum RandomImageSamplerMode
  {
    SINGLE_ACCEPTANCE_RATE,                ///< Sample all labels with a single acceptance rate.
    CLASS_DEPENDEND_ACCEPTANCE_RATE,       ///< Sample each class with an individual acceptance rate.
    SINGLE_NUMBER_OF_ACCEPTANCE,           ///< Draw a fixed total number of samples.
    CLASS_DEPENDEND_NUMBER_OF_ACCEPTANCE   ///< Draw a fixed number of samples per class.
  };

  /**
   * \brief Randomly samples voxels from a label image using configurable strategies.
   *
   * This filter produces a sub-sampled version of a label image. It supports
   * four sampling modes: single acceptance rate, class-dependent acceptance rate,
   * fixed number of samples, and class-dependent fixed number of samples.
   */
  class MITKCLUTILITIES_EXPORT RandomImageSampler : public ImageToImageFilter
  {
  public:

    mitkClassMacro( RandomImageSampler , ImageToImageFilter );
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);
    itkSetMacro(SamplingMode, RandomImageSamplerMode);
    itkGetConstMacro(SamplingMode, RandomImageSamplerMode);

    itkSetMacro(AcceptanceRate, double);
    itkGetConstMacro(AcceptanceRate, double);

    /**
     * \brief Set per-class acceptance rates for CLASS_DEPENDEND_ACCEPTANCE_RATE mode.
     * \param arg Vector of acceptance rates, one per class label.
     */
    void SetAcceptanceRateVector(std::vector<double> arg)
    {
      m_AcceptanceRateVector = arg;
    }

    itkGetConstMacro(AcceptanceRateVector, std::vector<double>);

    itkSetMacro(NumberOfSamples, unsigned int);
    itkGetConstMacro(NumberOfSamples, unsigned int);

    /**
     * \brief Set per-class sample counts for CLASS_DEPENDEND_NUMBER_OF_ACCEPTANCE mode.
     * \param arg Vector of sample counts, one per class label.
     */
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
    ~RandomImageSampler() override;
    /*!
    \brief Method generating the output information of this filter (e.g. image dimension, image type, etc.).
    The interface ImageToImageFilter requires this implementation. Everything is taken from the input image.
    */
    void GenerateOutputInformation() override;
    /*!
    \brief Method generating the output of this filter. Called in the updated process of the pipeline.
    This method generates the smoothed output image.
    */
    void GenerateData() override;

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
