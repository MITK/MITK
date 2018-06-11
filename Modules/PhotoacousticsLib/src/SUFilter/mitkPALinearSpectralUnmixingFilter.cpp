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
  m_PropertyCalculatorEigen = mitk::pa::PropertyCalculator::New();
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
    Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> Test = CalculateEndmemberMatrix(m_Chromophore, m_Wavelength);
    resultVector = Test.householderQr().solve(inputVector);
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

//Matrix with #chromophores colums and #wavelengths rows
//so Matrix Element (i,j) contains the Absorbtion of chromophore j @ wavelength i
Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> mitk::pa::LinearSpectralUnmixingFilter::CalculateEndmemberMatrix(
  std::vector<mitk::pa::PropertyCalculator::ChromophoreType> m_Chromophore, std::vector<int> m_Wavelength)
{
  unsigned int numberOfChromophores = m_Chromophore.size(); //columns
  unsigned int numberOfWavelengths = m_Wavelength.size(); //rows
  Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> EndmemberMatrixEigen(numberOfWavelengths, numberOfChromophores);

  for (unsigned int j = 0; j < numberOfChromophores; ++j)
  {
    if (m_Chromophore[j] == mitk::pa::PropertyCalculator::ChromophoreType::OXYGENATED)
    {
      for (unsigned int i = 0; i < numberOfWavelengths; ++i)
        EndmemberMatrixEigen(i, j) = propertyElement(m_Chromophore[j], m_Wavelength[i]);
    }
    else if (m_Chromophore[j] == mitk::pa::PropertyCalculator::ChromophoreType::DEOXYGENATED)
    {
      for (unsigned int i = 0; i < numberOfWavelengths; ++i)
        EndmemberMatrixEigen(i, j) = propertyElement(m_Chromophore[j], m_Wavelength[i]);
    }
    else if (m_Chromophore[j] == mitk::pa::PropertyCalculator::ChromophoreType::MELANIN)
    {
      for (unsigned int i = 0; i < numberOfWavelengths; ++i)
        EndmemberMatrixEigen(i, j) = propertyElement(m_Chromophore[j], m_Wavelength[i]);
    }
    else if (m_Chromophore[j] == mitk::pa::PropertyCalculator::ChromophoreType::ONEENDMEMBER)
    {
      for (unsigned int i = 0; i < numberOfWavelengths; ++i)
        EndmemberMatrixEigen(i, j) = 1;
    }
    else
      mitkThrow() << "404 CHROMOPHORE NOT FOUND!";
  }
  //MITK_INFO << "GENERATING ENMEMBERMATRIX [DONE]!";
  return EndmemberMatrixEigen;
}

float mitk::pa::LinearSpectralUnmixingFilter::propertyElement(mitk::pa::PropertyCalculator::ChromophoreType Chromophore, int Wavelength)
{
  float value = m_PropertyCalculatorEigen->GetAbsorptionForWavelength(Chromophore, Wavelength);
  if (value == 0)
    mitkThrow() << "WAVELENGTH " << Wavelength << "nm NOT SUPPORTED!";
  return value;
}
