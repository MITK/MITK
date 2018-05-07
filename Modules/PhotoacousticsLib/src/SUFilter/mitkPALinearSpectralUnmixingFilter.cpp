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


Eigen::VectorXf mitk::pa::LinearSpectralUnmixingFilter::SpectralUnmixingAlgorithm(
  Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> EndmemberMatrix, Eigen::VectorXf inputVector)
{

  //test other solvers https://eigen.tuxfamily.org/dox/group__TutorialLinearAlgebra.html

  int choseSolver = 3;

  bool relativErrorBool = false;
  bool tresholdBool = false;
  int treshold = 0;
  int defaultValue = -1;

  Eigen::Vector2f resultVector;

  if (choseSolver == 0)
  {
    resultVector = EndmemberMatrix.colPivHouseholderQr().solve(inputVector); //not working
  }

  if (choseSolver == 1)
  {
    resultVector = EndmemberMatrix.llt().solve(inputVector);
  }

  if (choseSolver == 2)
  {
    resultVector = EndmemberMatrix.householderQr().solve(inputVector);
  }

  if (choseSolver == 3)
  {
    resultVector = EndmemberMatrix.ldlt().solve(inputVector); //until now 'best' algorithm
  }

  if (relativErrorBool)
  {
    double relativeError = (EndmemberMatrix*inputVector - resultVector).norm() / resultVector.norm(); // norm() is L2 norm
    MITK_INFO << "rel err: " << relativeError;
  }

  //Set threshold and replace with default value if under threshold
  if (tresholdBool)
  {
    for (int i = 0; i < 2; ++i)
    {
      if (resultVector[i] < treshold)
      {
        resultVector[i] = defaultValue;
        MITK_INFO << "UNMIXING RESULT N/A";
      }
    }
  }

  return resultVector;
}
