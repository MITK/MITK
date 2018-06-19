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

void mitk::pa::LinearSpectralUnmixingFilter::SetAlgorithm(mitk::pa::LinearSpectralUnmixingFilter::AlgortihmType inputAlgorithmName)
{
  algorithmName = inputAlgorithmName;
}

Eigen::VectorXf mitk::pa::LinearSpectralUnmixingFilter::SpectralUnmixingAlgorithm(
  Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> EndmemberMatrix, Eigen::VectorXf inputVector)
{
  Eigen::VectorXf resultVector;

  if (mitk::pa::LinearSpectralUnmixingFilter::AlgortihmType::HOUSEHOLDERQR == algorithmName)
    resultVector = EndmemberMatrix.householderQr().solve(inputVector);

  else if (mitk::pa::LinearSpectralUnmixingFilter::AlgortihmType::LDLT == algorithmName)
  {
    mitkThrow() << "algorithm not working";
    resultVector = EndmemberMatrix.ldlt().solve(inputVector);
  }

  else if (mitk::pa::LinearSpectralUnmixingFilter::AlgortihmType::LLT == algorithmName)
    resultVector = EndmemberMatrix.llt().solve(inputVector);

  else if (mitk::pa::LinearSpectralUnmixingFilter::AlgortihmType::COLPIVHOUSEHOLDERQR == algorithmName)
    resultVector = EndmemberMatrix.colPivHouseholderQr().solve(inputVector);

  else if (mitk::pa::LinearSpectralUnmixingFilter::AlgortihmType::JACOBISVD == algorithmName)
    resultVector = EndmemberMatrix.jacobiSvd(Eigen::ComputeFullU | Eigen::ComputeFullV).solve(inputVector);

  else if (mitk::pa::LinearSpectralUnmixingFilter::AlgortihmType::FULLPIVLU == algorithmName)
    resultVector = EndmemberMatrix.fullPivLu().solve(inputVector);

  else if (mitk::pa::LinearSpectralUnmixingFilter::AlgortihmType::FULLPIVHOUSEHOLDERQR == algorithmName)
    resultVector = EndmemberMatrix.fullPivHouseholderQr().solve(inputVector);
  else
    mitkThrow() << "404 VIGRA ALGORITHM NOT FOUND";

  /*double relativeError = (EndmemberMatrix*resultVector - inputVector).norm() / inputVector.norm(); // norm() is L2 norm
  MITK_INFO << "relativ error: " << relativeError;
  float accuracyLevel = .1;
  bool resultIsApprox = inputVector.isApprox(EndmemberMatrix*resultVector, accuracyLevel);
  MITK_INFO << "IS APPROX RESULT: " << resultIsApprox;*/
  //MITK_INFO << "Result vector: " << resultVector;
  //++++++++++++++++++Maybe add this as additional ouptu image++++++++++++++++++++++
  return resultVector;
}
