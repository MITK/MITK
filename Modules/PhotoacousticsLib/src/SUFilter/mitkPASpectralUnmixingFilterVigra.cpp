/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkPASpectralUnmixingFilterVigra.h"

// ImageAccessor
#include <mitkImageReadAccessor.h>
#include <mitkImageWriteAccessor.h>

//vigra
#include <vigra/matrix.hxx>
#include <vigra/regression.hxx>
#include <vigra/quadprog.hxx>

mitk::pa::SpectralUnmixingFilterVigra::SpectralUnmixingFilterVigra()
{
}

mitk::pa::SpectralUnmixingFilterVigra::~SpectralUnmixingFilterVigra()
{
}

void mitk::pa::SpectralUnmixingFilterVigra::SetAlgorithm(mitk::pa::SpectralUnmixingFilterVigra::VigraAlgortihmType inputAlgorithmName)
{
  algorithmName = inputAlgorithmName;
}

void mitk::pa::SpectralUnmixingFilterVigra::AddWeight(unsigned int weight)
{
  double value = double(weight) / 100.0;
  weightsvec.push_back(value);
}

Eigen::VectorXf mitk::pa::SpectralUnmixingFilterVigra::SpectralUnmixingAlgorithm(
  Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> endmemberMatrix, Eigen::VectorXf inputVector)
{
  unsigned int numberOfWavelengths = endmemberMatrix.rows();
  unsigned int numberOfChromophores = endmemberMatrix.cols();

  // writes endmemberMatrix and inputVector into vigra::Matrix<double>
  std::vector<double> aData;
  std::vector<double> bData;
  for (unsigned int i = 0; i < numberOfWavelengths; ++i)
  {
    bData.push_back((double)inputVector(i));
    for (unsigned int j = 0; j < numberOfChromophores; ++j)
      aData.push_back((double)endmemberMatrix(i, j));
  }
  const double* aDat = aData.data();
  const double* bDat = bData.data();

  vigra::Matrix<double> A(vigra::Shape2(numberOfWavelengths, numberOfChromophores), aDat);
  vigra::Matrix<double> b(vigra::Shape2(numberOfWavelengths, 1), bDat);
  vigra::Matrix<double> x(vigra::Shape2(numberOfChromophores, 1));

  if (mitk::pa::SpectralUnmixingFilterVigra::VigraAlgortihmType::LARS == algorithmName)
    nonnegativeLeastSquares(A, b, x);

  else if (mitk::pa::SpectralUnmixingFilterVigra::VigraAlgortihmType::GOLDFARB == algorithmName)
  {
    vigra::linalg::Matrix<double> eye(vigra::linalg::identityMatrix<double>(numberOfChromophores)),
      zeros(vigra::Shape2(numberOfChromophores, 1)),
      empty,
      U = vigra::linalg::transpose(A)*A,
      // v= -transpose(A)*b replaced by -v used in "quadraticProgramming"
      v = vigra::linalg::transpose(A)*b;
      x = 0;
    quadraticProgramming(U, -v, empty, empty, eye, zeros, x);
  }

  else if (mitk::pa::SpectralUnmixingFilterVigra::VigraAlgortihmType::WEIGHTED == algorithmName)
  {
    if (weightsvec.size() != numberOfWavelengths)
      mitkThrow() << "Number of weights and wavelengths doesn't match! OR Invalid weight!";
    const double* weightsdat = weightsvec.data();
    vigra::Matrix<double> weigths(vigra::Shape2(numberOfWavelengths, 1), weightsdat);
    vigra::linalg::weightedLeastSquares(A, b, weigths, x);
  }

  else if (mitk::pa::SpectralUnmixingFilterVigra::VigraAlgortihmType::LS == algorithmName)
    linearSolve(A, b, x);

  else
    mitkThrow() << "404 VIGRA ALGORITHM NOT FOUND";

  Eigen::VectorXf resultVector(numberOfChromophores);
  for (unsigned int k = 0; k < numberOfChromophores; ++k)
    resultVector[k] = (float)x(k, 0);

  return resultVector;
}
