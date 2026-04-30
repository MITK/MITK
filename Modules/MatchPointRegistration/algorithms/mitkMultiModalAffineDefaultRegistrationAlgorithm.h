/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkMultiModalAffineDefaultRegistrationAlgorithm_h
#define mitkMultiModalAffineDefaultRegistrationAlgorithm_h

#include <mapDiscreteElements.h>
#include <mapITKAffineMattesMIMultiResRegistrationAlgorithmTemplate.h>
#include <mapConfigure.h>

#include <mitkMultiModalAffineDefaultRegistrationAlgorithm_ProfileResource.h>

namespace mitk
{

  /**
   * \brief Default multimodal affine registration algorithm for MITK.
   *
   * Provides a pre-configured ITK-based affine registration using Mattes Mutual Information
   * as the similarity metric with a multi-resolution strategy (3 levels). The algorithm is
   * designed for multimodal registration problem statements (e.g., CT to MR).
   *
   * Configuration details:
   * - Uses 3 resolution levels with adaptive optimizer scales and spatial sampling.
   * - Initializes via image centers (not center of gravity).
   * - Optimizer: Regular Step Gradient Descent (max step 3.0, min step 0.5, 200 iterations,
   *   relaxation factor 0.8).
   * - Metric: Mattes Mutual Information with 30 histogram bins. Level 0 uses all pixels;
   *   subsequent levels sample 30% of moving image pixels.
   *
   * \tparam TImageType The ITK image type for both fixed and moving images.
   *
   * \sa MultiModalRigidDefaultRegistrationAlgorithm, MultiModalTranslationDefaultRegistrationAlgorithm
   */
  template <class TImageType>
  class MultiModalAffineDefaultRegistrationAlgorithm :
    public map::algorithm::boxed::ITKAffineMattesMIMultiResRegistrationAlgorithm<TImageType, TImageType, ::map::algorithm::mitkMultiModalAffineDefaultRegistrationAlgorithmUIDPolicy, SealedFixedInterpolatorPolicyMacro< ::itk::LinearInterpolateImageFunction<TImageType, map::core::continuous::ScalarType> >, map::algorithm::itk::NoComponentInitializationPolicy>
  {
  public:
    typedef MultiModalAffineDefaultRegistrationAlgorithm Self;

    typedef map::algorithm::boxed::ITKAffineMattesMIMultiResRegistrationAlgorithm<TImageType, TImageType, ::map::algorithm::mitkMultiModalAffineDefaultRegistrationAlgorithmUIDPolicy, SealedFixedInterpolatorPolicyMacro< ::itk::LinearInterpolateImageFunction<TImageType, map::core::continuous::ScalarType> >, map::algorithm::itk::NoComponentInitializationPolicy>
      Superclass;

    typedef ::itk::SmartPointer<Self>                                     Pointer;
    typedef ::itk::SmartPointer<const Self>                               ConstPointer;

    itkTypeMacro(MultiModalAffineDefaultRegistrationAlgorithm,
      ITKAffineMattesMIMultiResRegistrationAlgorithm);
    mapNewAlgorithmMacro(Self);

  protected:
    MultiModalAffineDefaultRegistrationAlgorithm()
    {
    };

    ~MultiModalAffineDefaultRegistrationAlgorithm() override
    {
    };

    /**
     * \brief Configure the algorithm with default parameters.
     *
     * Sets 3 resolution levels, enables pre-initialization via image centers,
     * and configures optimizer step sizes, iteration count, relaxation factor,
     * and Mattes MI metric parameters (30 histogram bins, all pixels, explicit PDF derivatives).
     */
    void configureAlgorithm() override
    {
      Superclass::configureAlgorithm();

      this->setResolutionLevels(3);
      this->_preInitialize = true;
      this->_useCenterOfGravity = false;

      this->getConcreteOptimizerControl()->getConcreteOptimizer()->SetMaximumStepLength(3.00);
      this->getConcreteOptimizerControl()->getConcreteOptimizer()->SetMinimumStepLength(0.5);
      this->getConcreteOptimizerControl()->getConcreteOptimizer()->SetNumberOfIterations(200);
      this->getConcreteOptimizerControl()->getConcreteOptimizer()->SetRelaxationFactor(0.8);
      this->getConcreteOptimizerControl()->getConcreteOptimizer()->SetGradientMagnitudeTolerance(1e-4);

      //metric
      this->getConcreteMetricControl()->getConcreteMetric()->SetNumberOfHistogramBins(30);
      this->getConcreteMetricControl()->getConcreteMetric()->SetUseAllPixels(true);
      this->getConcreteMetricControl()->getConcreteMetric()->ReinitializeSeed();
      this->getConcreteMetricControl()->getConcreteMetric()->UseExplicitPDFDerivativesOn();
    }

    /**
     * \brief Adjust optimizer scales and spatial sampling between resolution levels.
     *
     * At level 0, matrix scale parameters are set to 10.0 and translation scales to 1/10000.
     * At higher levels, matrix scales are set to 1.0, translation scales to 1/1000, and
     * spatial sampling is reduced to 30% of the moving image pixels.
     */
    void
      doInterLevelSetup() override
    {
      Superclass::doInterLevelSetup();

      //scale setting
      int dimCount = TImageType::ImageDimension*TImageType::ImageDimension + TImageType::ImageDimension;
      int matrixEnd = TImageType::ImageDimension*TImageType::ImageDimension;
      typename Superclass::ConcreteOptimizerType::ScalesType scales(dimCount);
      double matrixScale = 1.0;
      double transScale = 1.0;

      if (this->getCurrentLevel() == 0)
      {
        matrixScale = 10.0;
        transScale = 1.0 / 10000;
      }
      else
      {
        matrixScale = 1.0;
        transScale = 1.0 / 1000;
      }

      for (int i = 0; i < dimCount; ++i)
      {
        if (i < matrixEnd)
        {
          scales[i] = matrixScale;

        }
        else
        {
          scales[i] = transScale;
        }
      }

      this->getConcreteOptimizerControl()->getConcreteOptimizer()->SetScales(scales);

      //spatial samples setting
      if (this->getCurrentLevel() != 0)
      {
        this->getConcreteMetricControl()->getConcreteMetric()->SetUseAllPixels(false);

        unsigned int nrOfSmpl = ::itk::Math::Round<unsigned int, double>
          (this->getMovingImage()->GetLargestPossibleRegion().GetNumberOfPixels() * 0.30);

        this->getConcreteMetricControl()->getConcreteMetric()->SetNumberOfSpatialSamples(nrOfSmpl);
      }
    };

  private:

    MultiModalAffineDefaultRegistrationAlgorithm(const Self& source);  //purposely not implemented
    void operator=(const Self&); //purposely not implemented
  };

}

#endif
