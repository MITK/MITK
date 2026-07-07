/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkMultiModalRigidDefaultRegistrationAlgorithm_h
#define mitkMultiModalRigidDefaultRegistrationAlgorithm_h

#include <mapDiscreteElements.h>
#include <mapITKEuler3DMattesMIMultiResRegistrationAlgorithmTemplate.h>
#include <mapConfigure.h>

#include <mitkMultiModalRigidDefaultRegistrationAlgorithm_ProfileResource.h>

namespace mitk
{
  /**
   * \brief Default multimodal rigid registration algorithm for MITK.
   *
   * Provides a pre-configured ITK-based rigid (Euler 3D) registration using Mattes
   * Mutual Information as the similarity metric with a multi-resolution strategy
   * (3 levels). The algorithm is designed for multimodal registration problem statements
   * (e.g., CT to MR) with 6 degrees of freedom (3 rotation + 3 translation).
   *
   * Configuration details:
   * - Uses 3 resolution levels with adaptive optimizer scales and spatial sampling.
   * - Initializes via image centers (not center of gravity).
   * - Optimizer: Regular Step Gradient Descent (max step 3.0, min step 0.5, 200 iterations,
   *   relaxation factor 0.8). Rotation scales are 1.0 (10.0 at level 0), translation
   *   scales are 1/1000 (1/10000 at level 0).
   * - Metric: Mattes Mutual Information with 30 histogram bins. Level 0 uses all pixels;
   *   subsequent levels sample 15% of moving image pixels.
   *
   * \tparam TImageType The ITK image type for both fixed and moving images.
   *
   * \sa MultiModalAffineDefaultRegistrationAlgorithm, MultiModalTranslationDefaultRegistrationAlgorithm
   */
  template<class TImageType>
  class MultiModalRigidDefaultRegistrationAlgorithm :
    public ::map::algorithm::boxed::ITKEuler3DMattesMIMultiResRegistrationAlgorithm<TImageType, TImageType, ::map::algorithm::mitkMultiModalRigidDefaultRegistrationAlgorithmUIDPolicy, SealedFixedInterpolatorPolicyMacro< ::itk::LinearInterpolateImageFunction<TImageType, ::map::core::continuous::ScalarType> >, ::map::algorithm::itk::NoComponentInitializationPolicy>
  {
  public:
    typedef MultiModalRigidDefaultRegistrationAlgorithm Self;

    typedef ::map::algorithm::boxed::ITKEuler3DMattesMIMultiResRegistrationAlgorithm<TImageType, TImageType, ::map::algorithm::mitkMultiModalRigidDefaultRegistrationAlgorithmUIDPolicy, SealedFixedInterpolatorPolicyMacro< ::itk::LinearInterpolateImageFunction<TImageType, ::map::core::continuous::ScalarType> >, ::map::algorithm::itk::NoComponentInitializationPolicy>
      Superclass;

    typedef ::itk::SmartPointer<Self>                                     Pointer;
    typedef ::itk::SmartPointer<const Self>                               ConstPointer;

    itkTypeMacro(MultiModalRigidDefaultRegistrationAlgorithm,
      ITKEuler3DMattesMIMultiResRegistrationAlgorithm);
    mapNewAlgorithmMacro(Self);

  protected:
    MultiModalRigidDefaultRegistrationAlgorithm()
    {
    };

    ~MultiModalRigidDefaultRegistrationAlgorithm() override
    {
    };

    /**
     * \brief Configure the algorithm with default parameters.
     *
     * Sets 3 resolution levels, enables pre-initialization via image centers,
     * and configures optimizer scales (rotation 1.0, translation 1/1000), step sizes,
     * iteration count, relaxation factor, and Mattes MI metric parameters
     * (30 histogram bins, all pixels, explicit PDF derivatives).
     */
    void configureAlgorithm() override
    {
      Superclass::configureAlgorithm();

      this->setResolutionLevels(3);
      this->_preInitialize = true;
      this->_useCenterOfGravity = false;

      //optimizer
      typename Superclass::ConcreteOptimizerType::ScalesType scales(6);
      scales[0] = 1.0;
      scales[1] = 1.0;
      scales[2] = 1.0;
      scales[3] = 1.0 / 1000;
      scales[4] = 1.0 / 1000;
      scales[5] = 1.0 / 1000;

      this->getConcreteOptimizerControl()->getConcreteOptimizer()->SetScales(scales);
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
     * At level 0, rotation scales are set to 10.0 and translation scales to 1/10000.
     * At higher levels, rotation scales are set to 1.0, translation scales to 1/1000,
     * and spatial sampling is reduced to 15% of the moving image pixels.
     */
    void
      doInterLevelSetup() override
    {
      Superclass::doInterLevelSetup();

      if (this->getCurrentLevel() == 0)
      {
        typename Superclass::OptimizerBaseType::SVNLOptimizerBaseType::ScalesType scales(6);
        scales[0] = 10.0;
        scales[1] = 10.0;
        scales[2] = 10.0;
        scales[3] = 1.0 / 10000;
        scales[4] = 1.0 / 10000;
        scales[5] = 1.0 / 10000;
        this->getConcreteOptimizerControl()->getConcreteOptimizer()->SetScales(scales);
      }
      else
      {
        this->getConcreteMetricControl()->getConcreteMetric()->SetUseAllPixels(false);

        typename Superclass::OptimizerBaseType::SVNLOptimizerBaseType::ScalesType scales(6);
        scales[0] = 1.0;
        scales[1] = 1.0;
        scales[2] = 1.0;
        scales[3] = 1.0 / 1000;
        scales[4] = 1.0 / 1000;
        scales[5] = 1.0 / 1000;

        this->getConcreteOptimizerControl()->getConcreteOptimizer()->SetScales(scales);

        unsigned int nrOfSmpl = ::itk::Math::Round<unsigned int, double>
          (this->getMovingImage()->GetLargestPossibleRegion().GetNumberOfPixels() * 0.15);

        this->getConcreteMetricControl()->getConcreteMetric()->SetNumberOfSpatialSamples(nrOfSmpl);
      }
    };

  private:

    MultiModalRigidDefaultRegistrationAlgorithm(const Self& source);  //purposely not implemented
    void operator=(const Self&); //purposely not implemented
  };

}

#endif
