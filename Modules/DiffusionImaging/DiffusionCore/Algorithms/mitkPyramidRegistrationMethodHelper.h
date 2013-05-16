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

#ifndef MITKPYRAMIDREGISTRATIONMETHODHELPER_H
#define MITKPYRAMIDREGISTRATIONMETHODHELPER_H

#include <DiffusionCoreExports.h>

#include <itkCommand.h>

#include <itkRegularStepGradientDescentOptimizer.h>
#include <itkMattesMutualInformationImageToImageMetric.h>
#include <itkNormalizedCorrelationImageToImageMetric.h>

#include <itkAffineTransform.h>
#include <itkEuler3DTransform.h>

#include <itkMultiResolutionImageRegistrationMethod.h>

/**
 * @brief The PyramidOptControlCommand class stears the step lenght of the
 * gradient descent optimizer in multi-scale registration methods
 */
template <typename RegistrationType >
class PyramidOptControlCommand : public itk::Command
{
public:

  typedef itk::RegularStepGradientDescentOptimizer OptimizerType;

  itkNewMacro( PyramidOptControlCommand )

  void Execute(itk::Object *caller, const itk::EventObject & /*event*/)
  {
    RegistrationType* registration = dynamic_cast< RegistrationType* >( caller );

    if( registration->GetCurrentLevel() == 0 )
      return;

    OptimizerType* optimizer = dynamic_cast< OptimizerType* >(registration->GetOptimizer());

    optimizer->SetMaximumStepLength( optimizer->GetMaximumStepLength() * 0.25f );
    optimizer->SetMinimumStepLength( optimizer->GetMinimumStepLength() * 0.1f );
  }

  void Execute(const itk::Object * /*object*/, const itk::EventObject & /*event*/){}
};

#endif // MITKPYRAMIDREGISTRATIONMETHODHELPER_H
