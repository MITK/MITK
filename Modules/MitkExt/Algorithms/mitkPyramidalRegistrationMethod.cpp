/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date: 2009-05-28 17:19:30 +0200 (Thu, 28 May 2009) $
Version:   $Revision: 17495 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkPyramidalRegistrationMethod.h"


namespace mitk {

  PyramidalRegistrationMethod::PyramidalRegistrationMethod() : m_Observer(NULL), m_Interpolator(0)
  {
    m_OptimizerParameters = OptimizerParameters::New();
    m_TransformParameters = TransformParameters::New();
    m_MetricParameters = MetricParameters::New();
    m_ReferenceImage = Image::New();
    m_MovingMask = Image::New();
    m_FixedMask = Image::New();

    m_Preset = new mitk::RigidRegistrationTestPreset();

    bool succeed = m_Preset->LoadPreset();
    if(!succeed)
    {
       std::cout << "RigidRegistrationParameters.xml is empty or does not exist. There are no presets to select." << std::endl;
       return;
    }
    
    m_UseMask = false;
    m_BlurMovingImage = true;
    m_BlurFixedImage = true;

  }

  PyramidalRegistrationMethod::~PyramidalRegistrationMethod()
  {
 
  }

  void PyramidalRegistrationMethod::GenerateData()
  {
    if (this->GetInput())
    {
      AccessByItk(this->GetInput(), GenerateData2);
    }
  }

  void PyramidalRegistrationMethod::SetObserver(RigidRegistrationObserver::Pointer observer)
  {
    m_Observer = observer;
  }

  void PyramidalRegistrationMethod::SetInterpolator(int interpolator)
  {
    m_Interpolator = interpolator;
  }

  void PyramidalRegistrationMethod::SetReferenceImage(Image::Pointer fixedImage)
  {
    m_ReferenceImage = fixedImage;
    SetNthInput(1, m_ReferenceImage);
    Modified();
  }


  mitk::TransformParameters::Pointer PyramidalRegistrationMethod::ParseTransformParameters(itk::Array<double> transformValues)
  {
    mitk::TransformParameters::Pointer transformParameters = TransformParameters::New();

    itk::Array<double> initialParameters;
    if(transformParameters->GetInitialParameters().size())
    {
      initialParameters = transformParameters->GetInitialParameters();
    }
    transformParameters = mitk::TransformParameters::New();
    transformParameters->SetTransform(transformValues[0]);
   
    if(transformParameters->GetInitialParameters().size())
    {
      transformParameters->SetInitialParameters(initialParameters); 
    }
    
    // Set scales. Every type of transform has a different number of scales!!! 
    // TODO: Finish for al types of transform (or find a better solution)
    itk::Array<double> scales;
    if(transformValues[0] == mitk::TransformParameters::AFFINETRANSFORM) scales.SetSize(12);
    if(transformValues[0] == mitk::TransformParameters::TRANSLATIONTRANSFORM) scales.SetSize(3);
    
    for(unsigned int i = 0; i < scales.size(); i++)
    {
      scales[i] = transformValues[i+2];        
    }
    transformParameters->SetScales(scales);      
    transformParameters->SetTransformInitializerOn(false);
    
    // Use Scales      
    if(transformValues[1] == 1)
    {
      transformParameters->SetUseOptimizerScales(true);              
    } 

    return transformParameters; 
  }
  
  mitk::MetricParameters::Pointer PyramidalRegistrationMethod::ParseMetricParameters(itk::Array<double> metricValues)
  {
    mitk::MetricParameters::Pointer metricParameters = mitk::MetricParameters::New();

    metricParameters->SetMetric(metricValues[0]);
    metricParameters->SetComputeGradient(metricValues[1]);

    // Some things have to be checked for every metric individually
    if(metricValues[0] == mitk::MetricParameters::MUTUALINFORMATIONHISTOGRAMIMAGETOIMAGEMETRIC)
    {       
      metricParameters->SetNumberOfHistogramBinsMutualInformationHistogram(metricValues[2]);
    }   

    if(metricValues[0] == mitk::MetricParameters::MATTESMUTUALINFORMATIONIMAGETOIMAGEMETRIC)
    {
      metricParameters->SetSpatialSamplesMattesMutualInformation(metricValues[3]);
      metricParameters->SetNumberOfHistogramBinsMattesMutualInformation(metricValues[4]);
    }

    return metricParameters;
  }

  mitk::OptimizerParameters::Pointer PyramidalRegistrationMethod::ParseOptimizerParameters(itk::Array<double> optimizerValues)
  {
    mitk::OptimizerParameters::Pointer optimizerParameters = mitk::OptimizerParameters::New();

    optimizerParameters->SetOptimizer(optimizerValues[0]);
    optimizerParameters->SetMaximize(optimizerValues[1]); //should be when used with maximize mutual information for example

    if(optimizerValues[0] == mitk::OptimizerParameters::GRADIENTDESCENTOPTIMIZER)
    {        
      optimizerParameters->SetLearningRateGradientDescent(optimizerValues[2]);
      optimizerParameters->SetNumberOfIterationsGradientDescent(optimizerValues[3]);
    }

    if(optimizerValues[0] == mitk::OptimizerParameters::REGULARSTEPGRADIENTDESCENTOPTIMIZER)
    {
      cout << "use regularstepgradientdescent" << endl;
      optimizerParameters->SetGradientMagnitudeToleranceRegularStepGradientDescent(optimizerValues[2]);
      optimizerParameters->SetMinimumStepLengthRegularStepGradientDescent(optimizerValues[3]);
      optimizerParameters->SetMaximumStepLengthRegularStepGradientDescent(optimizerValues[4]);
      optimizerParameters->SetRelaxationFactorRegularStepGradientDescent(optimizerValues[5]);
      optimizerParameters->SetNumberOfIterationsRegularStepGradientDescent(optimizerValues[6]);       
    }  

    return optimizerParameters;    
  }

  void PyramidalRegistrationMethod::SetMovingMask(Image::Pointer movingMask)
  {
    m_MovingMask = movingMask;
    SetNthInput(3, m_MovingMask);
    Modified();
  }

  void PyramidalRegistrationMethod::SetFixedMask(Image::Pointer FixedMask)
  {
    m_FixedMask = FixedMask;
    SetNthInput(4, m_FixedMask);
    Modified();
  }
 

} // end namespace
