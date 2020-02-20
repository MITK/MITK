/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkPALinearSpectralUnmixingFilter.h"

// Testing algorithms
#include <eigen3/Eigen/src/SVD/JacobiSVD.h>

// ImageAccessor
#include <mitkImageReadAccessor.h>
#include <mitkImageWriteAccessor.h>

mitk::pa::LinearSpectralUnmixingFilter::LinearSpectralUnmixingFilter()
{
}

mitk::pa::LinearSpectralUnmixingFilter::~LinearSpectralUnmixingFilter()
{
}

void mitk::pa::LinearSpectralUnmixingFilter::SetAlgorithm(mitk::pa::LinearSpectralUnmixingFilter::AlgortihmType inputAlgorithmName)
{
  algorithmName = inputAlgorithmName;
}

Eigen::VectorXf mitk::pa::LinearSpectralUnmixingFilter::SpectralUnmixingAlgorithm(
  Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> endmemberMatrix, Eigen::VectorXf inputVector)
{
  Eigen::VectorXf resultVector;

  if (mitk::pa::LinearSpectralUnmixingFilter::AlgortihmType::HOUSEHOLDERQR == algorithmName)
    resultVector = endmemberMatrix.householderQr().solve(inputVector);

  else if (mitk::pa::LinearSpectralUnmixingFilter::AlgortihmType::LDLT == algorithmName)
  {
    Eigen::LLT<Eigen::MatrixXf> lltOfA(endmemberMatrix);
    if (lltOfA.info() == Eigen::NumericalIssue)
    {
      mitkThrow() << "Possibly non semi-positive definitie endmembermatrix!";
    }
    else
      resultVector = endmemberMatrix.ldlt().solve(inputVector);
  }

  else if (mitk::pa::LinearSpectralUnmixingFilter::AlgortihmType::LLT == algorithmName)
  {
    Eigen::LLT<Eigen::MatrixXf> lltOfA(endmemberMatrix);
    if (lltOfA.info() == Eigen::NumericalIssue)
    {
      mitkThrow() << "Possibly non semi-positive definitie endmembermatrix!";
    }
    else
      resultVector = endmemberMatrix.llt().solve(inputVector);
  }

  else if (mitk::pa::LinearSpectralUnmixingFilter::AlgortihmType::COLPIVHOUSEHOLDERQR == algorithmName)
    resultVector = endmemberMatrix.colPivHouseholderQr().solve(inputVector);

  else if (mitk::pa::LinearSpectralUnmixingFilter::AlgortihmType::JACOBISVD == algorithmName)
    resultVector = endmemberMatrix.jacobiSvd(Eigen::ComputeFullU | Eigen::ComputeFullV).solve(inputVector);

  else if (mitk::pa::LinearSpectralUnmixingFilter::AlgortihmType::FULLPIVLU == algorithmName)
    resultVector = endmemberMatrix.fullPivLu().solve(inputVector);

  else if (mitk::pa::LinearSpectralUnmixingFilter::AlgortihmType::FULLPIVHOUSEHOLDERQR == algorithmName)
    resultVector = endmemberMatrix.fullPivHouseholderQr().solve(inputVector);
  else
    mitkThrow() << "404 VIGRA ALGORITHM NOT FOUND";

  return resultVector;
}
