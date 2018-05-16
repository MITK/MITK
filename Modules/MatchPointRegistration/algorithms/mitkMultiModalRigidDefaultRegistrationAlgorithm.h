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

#ifndef mitkMultiModalRigidDefaultRegistrationAlgorithm_h
#define mitkMultiModalRigidDefaultRegistrationAlgorithm_h

#include "mapDiscreteElements.h"
#include "mapITKEuler3DMattesMIMultiResRegistrationAlgorithmTemplate.h"
#include "mapConfigure.h"

#include "mitkMultiModalRigidDefaultRegistrationAlgorithm_ProfileResource.h"

namespace mitk
{
  /** \class MultiModalRigidDefaultRegistrationAlgorithm
  * Algorithm is used as default solution for multimodal rigid problem statements in DIPP.
  * Uses 3 Resolution levels. By default initializes via image centers
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
