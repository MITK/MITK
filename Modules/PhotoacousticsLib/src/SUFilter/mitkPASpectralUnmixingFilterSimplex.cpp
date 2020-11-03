/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
  Eigen::VectorXf normalizedInputVector(EndmemberMatrix.rows());
  normalizedInputVector = Normalization(EndmemberMatrix, inputVector);
  //normalizedInputVector = inputVector;


  float VolumeMax = simplexVolume(EndmemberMatrix);
  for (int i = 0; i < numberOfChromophores; ++i)
  {
    Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> A = GenerateA(EndmemberMatrix, normalizedInputVector, i);
    float Volume = simplexVolume(A);


    resultVector[i] = Volume / VolumeMax;

    myfile << "resultVector["<<i<<"]: " << resultVector[i] << "\n";
    myfile << "Volume: " << Volume << "\n";
    myfile << "VolumeMax: " << VolumeMax << "\n";

  }
  //

  return resultVector;
  // see code @ linearSUFilter
}


Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> mitk::pa::SpectralUnmixingFilterSimplex::GenerateA
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

Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> mitk::pa::SpectralUnmixingFilterSimplex::GenerateD2
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

float  mitk::pa::SpectralUnmixingFilterSimplex::simplexVolume(Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> Matrix)
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

int mitk::pa::SpectralUnmixingFilterSimplex::factorial(int n)
{
  if (n == 1)
    return 1;
  else
    return n * factorial(n - 1);
}

Eigen::VectorXf mitk::pa::SpectralUnmixingFilterSimplex::Normalization(
  Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> EndmemberMatrix, Eigen::VectorXf inputVector)
{
  int numberOfWavelengths = inputVector.rows();
  Eigen::VectorXf result(numberOfWavelengths);
  float NormalizationFactor = 1;
  float foo;
  float norm = 0;

  for (int i = 0; i < numberOfWavelengths; ++i)
  {
    foo = EndmemberMatrix(i, 0) - EndmemberMatrix(i, 1);
    if (std::abs(foo) > norm)
      norm = std::abs(foo);
  }

//ofstream myfile;
//myfile.open("SimplexNormalisation.txt");
  //NormalizationFactor = inputVector[0] * 2 / norm;
  myfile << "Normalizationfactor " << NormalizationFactor << "\n";

  for (int i = 0; i < numberOfWavelengths; ++i)
  {

    result[i]=(inputVector[i]/NormalizationFactor);
  }
//myfile.close();

  return result;
}
