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
#include "mapITKAffineMattesMIMultiResRegistrationAlgorithmTemplate.h"
#include "mapConfigure.h"

#include "MITK_MultiModal_affine_default_ProfileResource.h"

namespace mitk
{
  typedef map::core::discrete::Elements<3>::InternalImageType ImageType;

  /** \class MultiModalAffineDefaultRegistrationAlgorithm
  * Algorithm is used as default solution for multimodal affine problem statements in DIPP.
  * Uses 3 Resolution levels. By default initializes via image centers
  */
  class MultiModalAffineDefaultRegistrationAlgorithm :
    public map::algorithm::boxed::ITKAffineMattesMIMultiResRegistrationAlgorithm<ImageType, ImageType, ::map::algorithm::MITK_MultiModal_affine_defaultUIDPolicy, SealedFixedInterpolatorPolicyMacro< ::itk::LinearInterpolateImageFunction<ImageType, map::core::continuous::ScalarType> >, map::algorithm::itk::NoComponentInitializationPolicy>
  {
  public:
    typedef MultiModalAffineDefaultRegistrationAlgorithm Self;

    typedef ITKAffineMattesMIMultiResRegistrationAlgorithm<ImageType, ImageType, ::map::algorithm::MITK_MultiModal_affine_defaultUIDPolicy, SealedFixedInterpolatorPolicyMacro< ::itk::LinearInterpolateImageFunction<ImageType, map::core::continuous::ScalarType> >, map::algorithm::itk::NoComponentInitializationPolicy>
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

    virtual ~MultiModalAffineDefaultRegistrationAlgorithm()
    {
    };

    void configureAlgorithm()
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
      doInterLevelSetup()
    {
      Superclass::doInterLevelSetup();

      //scale setting
      int dimCount = ImageType::ImageDimension*ImageType::ImageDimension + ImageType::ImageDimension;
      int matrixEnd = ImageType::ImageDimension*ImageType::ImageDimension;
      Superclass::ConcreteOptimizerType::ScalesType scales(dimCount);
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

      getConcreteOptimizerControl()->getConcreteOptimizer()->SetScales(scales);

      //spatial samples setting
      if (this->getCurrentLevel() != 0)
      {
        getConcreteMetricControl()->getConcreteMetric()->SetUseAllPixels(false);

        unsigned int nrOfSmpl = ::itk::Math::Round<unsigned int, double>
          (this->getMovingImage()->GetLargestPossibleRegion().GetNumberOfPixels() * 0.30);

        getConcreteMetricControl()->getConcreteMetric()->SetNumberOfSpatialSamples(nrOfSmpl);
      }
    };

  private:

    MultiModalAffineDefaultRegistrationAlgorithm(const Self& source);  //purposely not implemented
    void operator=(const Self&); //purposely not implemented
  };

}

mapDeployAlgorithmMacro(mitk::MultiModalAffineDefaultRegistrationAlgorithm);
