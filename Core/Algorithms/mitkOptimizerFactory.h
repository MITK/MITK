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

  class OptimizerFactory
  {
  public:
    OptimizerFactory();
    ~OptimizerFactory();

    typedef itk::SingleValuedNonLinearOptimizer OptimizerType;

    OptimizerType::Pointer GetOptimizer();

    // for AmoebaOptimizer
    void SetNumberOfTransformParameters(int numberTransformParameters);

  protected:
    OptimizerParameters* m_OptimizerParameters;
    int m_NumberTransformParameters;
  };

} // namespace mitk

#endif // MITKOPTIMIZERFACTORY_H
