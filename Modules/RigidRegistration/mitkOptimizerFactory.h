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

#ifndef MITKOPTIMIZERFACTORY_H
#define MITKOPTIMIZERFACTORY_H

#include "itkSingleValuedNonLinearOptimizer.h"
#include "MitkRigidRegistrationExports.h"
#include "mitkOptimizerParameters.h"

namespace mitk {

  /*!
  \brief This class creates an optimizer for a rigid registration process.

  This class will e.g. be instantiated by mitkImageRegistrationMethod and an
  optimizer corresponding to the integer value stored in mitkOptimizerParameters
  will be created. Therefore SetOptimizerParameters() has to be called with an instance
  of mitkOptimizerParameters, which holds all parameter informations for the new
  optimizer.

  GetOptimizer() returns the optimizer which then can be used in combination with a
  transform, a metric and an interpolator within a registration pipeline.


  \ingroup RigidRegistration

  \author Daniel Stein
  */

  class MITKRIGIDREGISTRATION_EXPORT OptimizerFactory : public ::itk::Object
  {
  public:

    mitkClassMacro(OptimizerFactory, Object);
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    typedef itk::SingleValuedNonLinearOptimizer OptimizerType;

    /**
    \brief Returns the optimizer which then can be used in combination with a transform, a metric
    and an interpolator within a registration pipeline.
    */
    OptimizerType::Pointer GetOptimizer();

    /**
    \brief Sets the number of transformParameters.
    */
    void SetNumberOfTransformParameters(int numberTransformParameters);

    /**
    \brief Sets the instance to the optimizer parameters class which holds all parameters for the new optimizer.
    */
    void SetOptimizerParameters(OptimizerParameters::Pointer optimizerParameters)
    {
      m_OptimizerParameters = optimizerParameters;
    }

    /**
    \brief Returns the instance to the optimizer parameters class which holds all parameters for the new optimizer.
    */
    OptimizerParameters::Pointer GetOptimizerParameters()
    {
      return m_OptimizerParameters;
    }

  protected:

    OptimizerFactory();
    ~OptimizerFactory();

  private:
    OptimizerParameters::Pointer m_OptimizerParameters;
    int m_NumberTransformParameters;
  };

} // namespace mitk

#endif // MITKOPTIMIZERFACTORY_H
