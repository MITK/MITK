/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile: mitkImageToImageFilter.h,v $
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef MITKOPTIMIZERFACTORY_H
#define MITKOPTIMIZERFACTORY_H

#include "itkSingleValuedNonLinearOptimizer.h"
#include "mitkOptimizerParameters.h"

namespace mitk {

  class MITK_CORE_EXPORT OptimizerFactory : public ::itk::Object
  {
  public:

    mitkClassMacro(OptimizerFactory, Object);
    itkNewMacro(Self);

    typedef itk::SingleValuedNonLinearOptimizer OptimizerType;

    OptimizerType::Pointer GetOptimizer();

    // for AmoebaOptimizer
    void SetNumberOfTransformParameters(int numberTransformParameters);

    void SetOptimizerParameters(OptimizerParameters::Pointer optimizerParameters)
    {
      m_OptimizerParameters = optimizerParameters;
    }
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
