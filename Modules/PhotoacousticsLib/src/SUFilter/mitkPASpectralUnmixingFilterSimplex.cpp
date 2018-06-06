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

#include <mitkPASpectralUnmixingFilterSimplex.h>

// Includes for AddEnmemberMatrix
#include <eigen3/Eigen/Dense>

#include <cmath>

mitk::pa::SpectralUnmixingFilterSimplex::SpectralUnmixingFilterSimplex()
{

}

mitk::pa::SpectralUnmixingFilterSimplex::~SpectralUnmixingFilterSimplex()
{

}

Eigen::VectorXf mitk::pa::SpectralUnmixingFilterSimplex::SpectralUnmixingAlgorithm(
  Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> EndmemberMatrix, Eigen::VectorXf inputVector)
{
  int numberOfChromophores = EndmemberMatrix.cols();
  Eigen::VectorXf resultVector(numberOfChromophores);
  return resultVector;
  // see code @ linearSUFilter
}
