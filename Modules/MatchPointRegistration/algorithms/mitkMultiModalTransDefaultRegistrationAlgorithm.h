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

#ifndef mitkMultiModalTransDefaultRegistrationAlgorithm_h
#define mitkMultiModalTransDefaultRegistrationAlgorithm_h

#include "mapDiscreteElements.h"
#include "mapITKTransMattesMIMultiResRegistrationAlgorithm.h"
#include "mapConfigure.h"

#include "mitkMultiModalTransDefaultRegistrationAlgorithm_ProfileResource.h"

namespace mitk
{
        /** \class MultiModalTranslationDefaultRegistrationAlgorithm
        * Algorithm is used as default solution for multimodal 3-degrees of freedom (translation) problem statements in DIPP.
        * Uses 3 Resolution levels. By default initializes via image centers
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
