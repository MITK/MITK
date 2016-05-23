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

#include "mapDeploymentDLLHelper.h"
#include "mapDiscreteElements.h"
#include "mapITKEuler3DMattesMIMultiResRegistrationAlgorithmTemplate.h"
#include "mapConfigure.h"

#include "MITK_MultiModal_rigid_default_ProfileResource.h"

namespace mitk
{
                typedef map::core::discrete::Elements<3>::InternalImageType ImageType;

                /** \class MultiModalRigidDefaultRegistrationAlgorithm
                * Algorithm is used as default solution for multimodal rigid problem statements in DIPP.
                * Uses 3 Resolution levels. By default initializes via image centers
                */
                class MultiModalRigidDefaultRegistrationAlgorithm :
          public map::algorithm::boxed::ITKEuler3DMattesMIMultiResRegistrationAlgorithm<ImageType, ImageType, ::map::algorithm::MITK_MultiModal_rigid_defaultUIDPolicy, SealedFixedInterpolatorPolicyMacro< ::itk::LinearInterpolateImageFunction<ImageType, map::core::continuous::ScalarType> >, map::algorithm::itk::NoComponentInitializationPolicy>
                {
                public:
                    typedef MultiModalRigidDefaultRegistrationAlgorithm Self;

          typedef ITKEuler3DMattesMIMultiResRegistrationAlgorithm<ImageType, ImageType, ::map::algorithm::MITK_MultiModal_rigid_defaultUIDPolicy, SealedFixedInterpolatorPolicyMacro< ::itk::LinearInterpolateImageFunction<ImageType, map::core::continuous::ScalarType> >, map::algorithm::itk::NoComponentInitializationPolicy>
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

                    virtual ~MultiModalRigidDefaultRegistrationAlgorithm()
                    {
                    };

                    void configureAlgorithm()
                    {
                        Superclass::configureAlgorithm();

                        this->setResolutionLevels(3);
                        this->_preInitialize = true;
                        this->_useCenterOfGravity = false;

                        //optimizer
                        ConcreteOptimizerType::ScalesType scales(6);
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

                    void
                    doInterLevelSetup()
                    {
                        Superclass::doInterLevelSetup();

                        if (this->getCurrentLevel() == 0)
                        {
                            OptimizerBaseType::SVNLOptimizerBaseType::ScalesType scales(6);
                            scales[0] = 10.0;
                            scales[1] = 10.0;
                            scales[2] = 10.0;
                            scales[3] = 1.0 / 10000;
                            scales[4] = 1.0 / 10000;
                            scales[5] = 1.0 / 10000;
                            getConcreteOptimizerControl()->getConcreteOptimizer()->SetScales(scales);
                        }
                        else
                        {
                            getConcreteMetricControl()->getConcreteMetric()->SetUseAllPixels(false);

                            OptimizerBaseType::SVNLOptimizerBaseType::ScalesType scales(6);
                            scales[0] = 1.0;
                            scales[1] = 1.0;
                            scales[2] = 1.0;
                            scales[3] = 1.0 / 1000;
                            scales[4] = 1.0 / 1000;
                            scales[5] = 1.0 / 1000;

                            getConcreteOptimizerControl()->getConcreteOptimizer()->SetScales(scales);

                            unsigned int nrOfSmpl = ::itk::Math::Round<unsigned int, double>
                                                    (this->getMovingImage()->GetLargestPossibleRegion().GetNumberOfPixels() * 0.15);

                            getConcreteMetricControl()->getConcreteMetric()->SetNumberOfSpatialSamples(nrOfSmpl);
                        }
                    };

                private:

                    MultiModalRigidDefaultRegistrationAlgorithm(const Self& source);  //purposely not implemented
                    void operator=(const Self&); //purposely not implemented
                };

}

mapDeployAlgorithmMacro(mitk::MultiModalRigidDefaultRegistrationAlgorithm);
