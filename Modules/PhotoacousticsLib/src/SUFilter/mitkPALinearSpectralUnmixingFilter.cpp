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
  Eigen::VectorXf resultVector;

  if (mitk::pa::LinearSpectralUnmixingFilter::AlgortihmType::colPivHouseholderQr == algorithmIndex)
    resultVector = EndmemberMatrix.colPivHouseholderQr().solve(inputVector);

  if (mitk::pa::LinearSpectralUnmixingFilter::AlgortihmType::llt == algorithmIndex)
    resultVector = EndmemberMatrix.llt().solve(inputVector);

  if (mitk::pa::LinearSpectralUnmixingFilter::AlgortihmType::householderQr == algorithmIndex)
    resultVector = EndmemberMatrix.householderQr().solve(inputVector);

  if (mitk::pa::LinearSpectralUnmixingFilter::AlgortihmType::ldlt == algorithmIndex)
  {
    mitkThrow() << "algorithm not working";
    resultVector = EndmemberMatrix.ldlt().solve(inputVector);
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
    mitkThrow() << "404 NO ALGORITHM IMPLEMENTED";
  }

  /*double relativeError = (EndmemberMatrix*resultVector - inputVector).norm() / inputVector.norm(); // norm() is L2 norm
  MITK_INFO << "relativ error: " << relativeError;
  float accuracyLevel = .1;
  bool resultIsApprox = inputVector.isApprox(EndmemberMatrix*resultVector, accuracyLevel);
  MITK_INFO << "IS APPROX RESULT: " << resultIsApprox;*/
  //MITK_INFO << "Result vector: " << resultVector;
  //++++++++++++++++++Maybe add this as additional ouptu image++++++++++++++++++++++
  return resultVector;
}
