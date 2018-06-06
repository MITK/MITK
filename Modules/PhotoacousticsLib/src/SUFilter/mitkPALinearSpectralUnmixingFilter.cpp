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
  Eigen::Vector2f resultVector;

  if (mitk::pa::LinearSpectralUnmixingFilter::AlgortihmType::colPivHouseholderQr == algorithmIndex)
    resultVector = EndmemberMatrix.colPivHouseholderQr().solve(inputVector);

  if (mitk::pa::LinearSpectralUnmixingFilter::AlgortihmType::llt == algorithmIndex)
    resultVector = EndmemberMatrix.llt().solve(inputVector);


  if (mitk::pa::LinearSpectralUnmixingFilter::AlgortihmType::householderQr == algorithmIndex)
    resultVector = EndmemberMatrix.householderQr().solve(inputVector);

  if (mitk::pa::LinearSpectralUnmixingFilter::AlgortihmType::ldlt == algorithmIndex)
  {
    mitkThrow() << "algorithm not working";
    resultVector = EndmemberMatrix.ldlt().solve(inputVector); //not working because matrix not quadratic(?)
  }

  if (mitk::pa::LinearSpectralUnmixingFilter::AlgortihmType::jacobiSvd == algorithmIndex)
    resultVector = EndmemberMatrix.jacobiSvd(Eigen::ComputeFullU | Eigen::ComputeFullV).solve(inputVector);

  if (mitk::pa::LinearSpectralUnmixingFilter::AlgortihmType::fullPivLu == algorithmIndex)
    resultVector = EndmemberMatrix.fullPivLu().solve(inputVector);

  if (mitk::pa::LinearSpectralUnmixingFilter::AlgortihmType::householderQr == algorithmIndex)
    resultVector = EndmemberMatrix.householderQr().solve(inputVector);

  if (mitk::pa::LinearSpectralUnmixingFilter::AlgortihmType::fullPivHouseholderQr == algorithmIndex)
    resultVector = EndmemberMatrix.fullPivHouseholderQr().solve(inputVector);

  if (mitk::pa::LinearSpectralUnmixingFilter::AlgortihmType::test == algorithmIndex)
  {
    int numberOfChromophores = EndmemberMatrix.cols();

    float VolumeMax = simplexVolume(EndmemberMatrix);
    for (int i = 0; i < numberOfChromophores; ++i)
    {
      Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> A = GenerateA(EndmemberMatrix, inputVector, i);
      float Volume = simplexVolume(A);
      //MITK_INFO << "VolumeMax: " << VolumeMax;
      //MITK_INFO << "Volume: " << Volume;
      //MITK_INFO << "Result vector[i]: " << Volume / VolumeMax;
      resultVector[i] = Volume / VolumeMax;
    }
    //mitkThrow() << "404";
  }

  /*double relativeError = (EndmemberMatrix*resultVector - inputVector).norm() / inputVector.norm(); // norm() is L2 norm
  MITK_INFO << "relativ error: " << relativeError;
  float accuracyLevel = .1;
  bool resultIsApprox = inputVector.isApprox(EndmemberMatrix*resultVector, accuracyLevel);
  MITK_INFO << "IS APPROX RESULT: " << resultIsApprox;*/
  //MITK_INFO << "Result vector: " << resultVector;
  return resultVector;
}

/* look at: https://eigen.tuxfamily.org/dox/classEigen_1_1FullPivLU.html#af563471f6f3283fd10779ef02dd0b748 
This method just tries to find as good a solution as possible. If you want to check whether a solution exists or if
it is accurate, just call this function to get a result and then compute the error of this result, or use MatrixBase::isApprox()
directly, for instance like this: bool a_solution_exists = (A*result).isApprox(b, precision);
This method avoids dividing by zero, so that the non-existence of a solution doesn't by itself mean that you'll get inf or nan values.
If there exists more than one solution, this method will arbitrarily choose one.
If you need a complete analysis of the space of solutions, take the one solution obtained by this method and add to it
elements of the kernel, as determined by kernel().*/


Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> mitk::pa::LinearSpectralUnmixingFilter::GenerateA
(Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> EndmemberMatrix, Eigen::VectorXf inputVector, int i)
{
  Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> A = EndmemberMatrix;
  int numberOfChromophores = EndmemberMatrix.cols();

  for (int j = 0; j < numberOfChromophores; ++j)
  {
    A(i, j) = inputVector(j);
  }

  return A;
}

Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> mitk::pa::LinearSpectralUnmixingFilter::GenerateD2
(Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> A)
{
  int numberOfChromophores = A.cols();

  Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> D2(numberOfChromophores, numberOfChromophores);

  for (int i = 0; i < numberOfChromophores; ++i)
  {
    for (int j = 0; j < numberOfChromophores; ++j)
    {
      Eigen::VectorXf x = A.col(i) - A.col(j);
      //MITK_INFO << "a_col_i: " <<A.col(i);
      //MITK_INFO << "a_col_j: " <<A.col(j);
      //MITK_INFO << "x: " <<x;
      Eigen::VectorXf y = x;
      float foo = x.dot(y);
      //MITK_INFO << "x*y: " << foo;

      D2(i, j) = foo;
    }
  }

  return D2;
}

float  mitk::pa::LinearSpectralUnmixingFilter::simplexVolume(Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> Matrix)
{
  float Volume;
  int numberOfChromophores = Matrix.cols();
  Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> C(numberOfChromophores + 1, numberOfChromophores + 1);
  Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> D2 = GenerateD2(Matrix);

  for (int i = 0; i < numberOfChromophores; ++i)
  {
    for (int j = 0; j < numberOfChromophores; ++j)
    {
      C(i, j) = D2(i, j);
    }
    C(i, numberOfChromophores) = 1;
    for (int k = 0; k < numberOfChromophores; ++k)
    {
      C(numberOfChromophores, k) = 1;
    }
    C(numberOfChromophores, numberOfChromophores) = 0;
  }

  float detC = -C.determinant();// determinate von C
  float denominator = (factorial(numberOfChromophores - 1)) ^ 2 * 2 ^ (numberOfChromophores - 1)*(-1) ^ numberOfChromophores;
  Volume = std::sqrt(detC / denominator);
  //MITK_INFO << "detC: " << detC;

  //MITK_INFO << "denominator: " << denominator;

  //MITK_INFO << "Volume: " << Volume;

  return Volume;
}

int mitk::pa::LinearSpectralUnmixingFilter::factorial(int n)
{
  if (n == 1)
    return 1;
  else
    return n * factorial(n - 1);
}
