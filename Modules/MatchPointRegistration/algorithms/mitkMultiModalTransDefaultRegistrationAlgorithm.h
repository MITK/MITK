/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkMultiModalTransDefaultRegistrationAlgorithm_h
#define mitkMultiModalTransDefaultRegistrationAlgorithm_h

#include <mapDiscreteElements.h>
#include <mapITKTransMattesMIMultiResRegistrationAlgorithm.h>
#include <mapConfigure.h>

#include <mitkMultiModalTransDefaultRegistrationAlgorithm_ProfileResource.h>

namespace mitk
{
        /**
         * \brief Default multimodal translation-only registration algorithm for MITK.
         *
         * Provides a pre-configured ITK-based translation registration using Mattes
         * Mutual Information as the similarity metric with a multi-resolution strategy
         * (3 levels). The algorithm is designed for multimodal registration problem statements
         * with only 3 degrees of freedom (translation along X, Y, Z).
         *
         * Configuration details:
         * - Uses 3 resolution levels with adaptive spatial sampling.
         * - Initializes via image centers (not center of gravity).
         * - Optimizer: Regular Step Gradient Descent (max step 3.0, min step 0.5, 200 iterations,
         *   relaxation factor 0.8). All 3 translation scales are set to 1.0.
         * - Metric: Mattes Mutual Information with 30 histogram bins. Level 0 uses all pixels;
         *   subsequent levels sample 15% of moving image pixels.
         *
         * \tparam TImageType The ITK image type for both fixed and moving images.
         *
         * \sa MultiModalAffineDefaultRegistrationAlgorithm, MultiModalRigidDefaultRegistrationAlgorithm
         */
        template<class TImageType>
        class MultiModalTranslationDefaultRegistrationAlgorithm :
          public map::algorithm::boxed::ITKTransMattesMIMultiResRegistrationAlgorithm<TImageType, TImageType, ::map::algorithm::mitkMultiModalTransDefaultRegistrationAlgorithmUIDPolicy, SealedFixedInterpolatorPolicyMacro< ::itk::LinearInterpolateImageFunction<TImageType, map::core::continuous::ScalarType> >, map::algorithm::itk::NoComponentInitializationPolicy>
        {
        public:
          typedef MultiModalTranslationDefaultRegistrationAlgorithm Self;

          typedef map::algorithm::boxed::ITKTransMattesMIMultiResRegistrationAlgorithm<TImageType, TImageType, ::map::algorithm::mitkMultiModalTransDefaultRegistrationAlgorithmUIDPolicy, SealedFixedInterpolatorPolicyMacro< ::itk::LinearInterpolateImageFunction<TImageType, map::core::continuous::ScalarType> >, map::algorithm::itk::NoComponentInitializationPolicy>
          Superclass;

          typedef ::itk::SmartPointer<Self>                                     Pointer;
          typedef ::itk::SmartPointer<const Self>                               ConstPointer;

          itkTypeMacro(MultiModalTranslationDefaultRegistrationAlgorithm,
                       ITKEuler3DMattesMIMultiResRegistrationAlgorithm);
          mapNewAlgorithmMacro(Self);

        protected:
          MultiModalTranslationDefaultRegistrationAlgorithm()
          {
          };

          ~MultiModalTranslationDefaultRegistrationAlgorithm() override
          {
          };

          /**
           * \brief Configure the algorithm with default parameters.
           *
           * Sets 3 resolution levels, enables pre-initialization via image centers,
           * and configures optimizer scales (all 1.0 for translation), step sizes,
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
            typename Superclass::ConcreteOptimizerType::ScalesType scales(3);
            scales[0] = 1.0;
            scales[1] = 1.0;
            scales[2] = 1.0;

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
           * \brief Adjust spatial sampling between resolution levels.
           *
           * At levels above 0, spatial sampling is reduced to 15% of the moving
           * image pixels for improved performance.
           */
          void
          doInterLevelSetup() override
          {
            Superclass::doInterLevelSetup();

            if (this->getCurrentLevel() != 0)
            {
              this->getConcreteMetricControl()->getConcreteMetric()->SetUseAllPixels(false);

              unsigned int nrOfSmpl = ::itk::Math::Round<unsigned int, double>
                                      (this->getMovingImage()->GetLargestPossibleRegion().GetNumberOfPixels() * 0.15);

              this->getConcreteMetricControl()->getConcreteMetric()->SetNumberOfSpatialSamples(nrOfSmpl);
            }
          };

        private:

          MultiModalTranslationDefaultRegistrationAlgorithm(const Self& source);  //purposely not implemented
          void operator=(const Self&); //purposely not implemented
        };

}

#endif
