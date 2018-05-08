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

#include "mitkPALinearSpectralUnmixingFilter.h"

// Includes for AddEnmemberMatrix
#include "mitkPAPropertyCalculator.h"
#include <eigen3/Eigen/Dense>

// Testing algorithms
#include <eigen3\Eigen\src\SVD\JacobiSVD.h>

// ImageAccessor
#include <mitkImageReadAccessor.h>
#include <mitkImageWriteAccessor.h>

mitk::pa::LinearSpectralUnmixingFilter::LinearSpectralUnmixingFilter()
{
 
}

mitk::pa::LinearSpectralUnmixingFilter::~LinearSpectralUnmixingFilter()
{

}

void mitk::pa::LinearSpectralUnmixingFilter::SetAlgorithm(int SetAlgorithmIndex)
{
  algorithmIndex = static_cast<mitk::pa::LinearSpectralUnmixingFilter::AlgortihmType>(SetAlgorithmIndex);
}

Eigen::VectorXf mitk::pa::LinearSpectralUnmixingFilter::SpectralUnmixingAlgorithm(
  Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> EndmemberMatrix, Eigen::VectorXf inputVector)
{

  //test other solvers https://eigen.tuxfamily.org/dox/group__TutorialLinearAlgebra.html

  Eigen::Vector2f resultVector;

  if (mitk::pa::LinearSpectralUnmixingFilter::AlgortihmType::colPivHouseholderQr == algorithmIndex)
  {
    resultVector = EndmemberMatrix.colPivHouseholderQr().solve(inputVector); //works :)
  }

  if (mitk::pa::LinearSpectralUnmixingFilter::AlgortihmType::llt == algorithmIndex)
  {
    resultVector = EndmemberMatrix.llt().solve(inputVector); //works with negativ values (no correct unmixing)
  }

  if (mitk::pa::LinearSpectralUnmixingFilter::AlgortihmType::householderQr == algorithmIndex)
  {
    resultVector = EndmemberMatrix.householderQr().solve(inputVector); //works :)
  }

  if (mitk::pa::LinearSpectralUnmixingFilter::AlgortihmType::ldlt == algorithmIndex)
  {
    mitkThrow() << "not working";
    resultVector = EndmemberMatrix.ldlt().solve(inputVector); //not working because matrix not quadratic(?)
  }

  if (mitk::pa::LinearSpectralUnmixingFilter::AlgortihmType::jacobiSvd == algorithmIndex)
  {
    mitkThrow() << "not working";
    resultVector = EndmemberMatrix.jacobiSvd().solve(inputVector); //not working
  }

  if (mitk::pa::LinearSpectralUnmixingFilter::AlgortihmType::fullPivLu == algorithmIndex)
  {
    resultVector = EndmemberMatrix.fullPivLu().solve(inputVector); //works :)
  }

  if (mitk::pa::LinearSpectralUnmixingFilter::AlgortihmType::householderQr == algorithmIndex)
  {
    resultVector = EndmemberMatrix.householderQr().solve(inputVector); //works :)
  }

  if (mitk::pa::LinearSpectralUnmixingFilter::AlgortihmType::fullPivHouseholderQr == algorithmIndex)
  {
    resultVector = EndmemberMatrix.fullPivHouseholderQr().solve(inputVector);//works :)
  }

  //testing new algorithms:
  if (mitk::pa::LinearSpectralUnmixingFilter::AlgortihmType::test == algorithmIndex)
  {
    mitkThrow() << "nothing implemented";
  }

  double relativeError = (EndmemberMatrix*resultVector - inputVector).norm() / inputVector.norm(); // norm() is L2 norm
  //MITK_INFO << "relativ error: " << relativeError;
  float accuracyLevel = .1;
  bool resultIsApprox = inputVector.isApprox(EndmemberMatrix*resultVector, accuracyLevel);
  //MITK_INFO << "IS APPROX RESULT: " << resultIsApprox;
  return resultVector;
}
