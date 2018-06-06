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

  float VolumeMax = simplexVolume(EndmemberMatrix);
  for (int i = 0; i < numberOfChromophores; ++i)
  {
    Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> A = GenerateA(EndmemberMatrix, inputVector, i);
    float Volume = simplexVolume(A);
    resultVector[i] = Volume / VolumeMax;
  }

  return resultVector;
}

Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> GenerateA(Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> EndmemberMatrix,
  Eigen::VectorXf inputVector, int i)
{
  Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> A = EndmemberMatrix;
  int numberOfChromophores = EndmemberMatrix.cols();

  for (int j = 0; j < numberOfChromophores; ++j)
  {
    A(i, j) = inputVector(j);
  }

  return A;
}

Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> GenerateD2(Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> A)
{
  Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> D2;
  int numberOfChromophores = A.cols();

  for (int i = 0; i < numberOfChromophores; ++i)
  {
    for (int j = 0; j < numberOfChromophores; ++j)
    {
      Eigen::VectorXf x = A.col(i) - A.col(j);
      D2(i, j) = x.dot(x);
    }
  }

  return D2;
}

float simplexVolume(Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> Matrix)
{
  float Volume;
  int numberOfChromophores = Matrix.cols();
  Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> C;
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

  float detC = C.determinant();// determinate von C
  float denominator = (factorial(numberOfChromophores-1) ^ 2 * 2 ^ (numberOfChromophores - 1)*(-1) ^ numberOfChromophores;

  return Volume;
}

int factorial(int n)
{
  if (n == 1)
    return 1;
  else
    return n * factorial(n - 1);
}
