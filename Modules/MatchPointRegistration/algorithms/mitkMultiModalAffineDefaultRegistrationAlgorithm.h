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

#ifndef mitkMultiModalAffineDefaultRegistrationAlgorithm_h
#define mitkMultiModalAffineDefaultRegistrationAlgorithm_h

#include "mapDiscreteElements.h"
#include "mapITKAffineMattesMIMultiResRegistrationAlgorithmTemplate.h"
#include "mapConfigure.h"

#include "mitkMultiModalAffineDefaultRegistrationAlgorithm_ProfileResource.h"

namespace mitk
{

  /** \class MultiModalAffineDefaultRegistrationAlgorithm
  * Algorithm is used as default solution for multimodal affine problem statements in DIPP.
  * Uses 3 Resolution levels. By default initializes via image centers
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
