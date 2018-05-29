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

void mitk::pa::LinearSpectralUnmixingFilter::SetAlgorithm(mitk::pa::LinearSpectralUnmixingFilter::AlgortihmType SetAlgorithmIndex)
{
  algorithmIndex = SetAlgorithmIndex;
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
    mitkThrow() << "algorithm not working";
    resultVector = EndmemberMatrix.ldlt().solve(inputVector); //not working because matrix not quadratic(?)
  }

  if (mitk::pa::LinearSpectralUnmixingFilter::AlgortihmType::jacobiSvd == algorithmIndex)
  {
    resultVector = EndmemberMatrix.jacobiSvd(Eigen::ComputeFullU | Eigen::ComputeFullV).solve(inputVector);
  }

  if (mitk::pa::LinearSpectralUnmixingFilter::AlgortihmType::fullPivLu == algorithmIndex)
  {
    resultVector = EndmemberMatrix.fullPivLu().solve(inputVector); //works :)
    // look at: https://eigen.tuxfamily.org/dox/classEigen_1_1FullPivLU.html#af563471f6f3283fd10779ef02dd0b748 
    /* ALSO FOR ALL QR METHODS
    This method just tries to find as good a solution as possible. If you want to check whether a solution exists or if 
    it is accurate, just call this function to get a result and then compute the error of this result, or use MatrixBase::isApprox() 
    directly, for instance like this: bool a_solution_exists = (A*result).isApprox(b, precision); 
    This method avoids dividing by zero, so that the non-existence of a solution doesn't by itself mean that you'll get inf or nan values.
    If there exists more than one solution, this method will arbitrarily choose one. 
    If you need a complete analysis of the space of solutions, take the one solution obtained by this method and add to it
    elements of the kernel, as determined by kernel().
    */
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
    //mitkThrow() << "nothing implemented";
    Eigen::LLT<Eigen::MatrixXf> lltOfA(EndmemberMatrix); // compute the Cholesky decomposition of A
    if (lltOfA.info() == Eigen::NumericalIssue)
      MITK_INFO << "not positive semi definite";
    else
      MITK_INFO << "semi definite";
    resultVector = EndmemberMatrix.fullPivHouseholderQr().solve(inputVector);//works :)
    //resultVector = EndmemberMatrix.completeOrthogonalDecomposition().solve(inputVector); //not a member of Eigen?
    //resultVector = EndmemberMatrix.bdcSvd().solve(inputVector); //not a member of Eigen?
  }

  /*double relativeError = (EndmemberMatrix*resultVector - inputVector).norm() / inputVector.norm(); // norm() is L2 norm
  MITK_INFO << "relativ error: " << relativeError;
  float accuracyLevel = .1;
  bool resultIsApprox = inputVector.isApprox(EndmemberMatrix*resultVector, accuracyLevel);
  MITK_INFO << "IS APPROX RESULT: " << resultIsApprox;*/
  return resultVector;
}
