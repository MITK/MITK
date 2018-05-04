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

#include "mitkPASpectralUnmixingFilter.h"

// Includes for AddEnmemberMatrix
#include "mitkPAPropertyCalculator.h"
#include <eigen3/Eigen/Dense>

// ImageAccessor
#include <mitkImageReadAccessor.h>
#include <mitkImageWriteAccessor.h>

// For testing reasons
#include <random>

mitk::pa::SpectralUnmixingFilter::SpectralUnmixingFilter()
{
  this->SetNumberOfIndexedOutputs(2);

  for (unsigned int i = 0; i<GetNumberOfIndexedOutputs(); i++)
  {
    this->SetNthOutput(i, mitk::Image::New());
  }

  m_PropertyCalculator = mitk::pa::PropertyCalculator::New();
}

mitk::pa::SpectralUnmixingFilter::~SpectralUnmixingFilter()
{

}

void mitk::pa::SpectralUnmixingFilter::AddWavelength(int wavelength)
{
  m_Wavelength.push_back(wavelength);
}

void mitk::pa::SpectralUnmixingFilter::AddChromophore(ChromophoreType chromophore)
{
  m_Chromophore.push_back(chromophore);
}

void mitk::pa::SpectralUnmixingFilter::GenerateData()
{
  MITK_INFO << "GENERATING DATA..";

  // Get input image
  mitk::Image::Pointer input = GetInput(0);

  unsigned int xDim = input->GetDimensions()[0];
  unsigned int yDim = input->GetDimensions()[1];
  unsigned int zDim = input->GetDimensions()[2];

  MITK_INFO << "x dimension: " << xDim;
  MITK_INFO << "y dimension: " << yDim;
  MITK_INFO << "z dimension: " << zDim;

  CheckPreConditions(zDim);
  InitializeOutputs();
  EndmemberMatrix = AddEndmemberMatrix();
  
  // Copy input image into array
  mitk::ImageReadAccessor readAccess(input);
  const float* inputDataArray = ((const float*)readAccess.GetData());

  //loop over every pixel @ x,y plane
  for (unsigned int x = 0; x < xDim; x++)
  {
    for (unsigned int y = 0; y < yDim; y++)
    {
      Eigen::VectorXf inputVector(zDim);
      for (unsigned int z = 0; z < zDim; z++)
      {
        // Get pixel value of pixel x,y @ wavelength z
        unsigned int pixelNumber = (xDim*yDim*z) + x * yDim + y;
        auto pixel = inputDataArray[pixelNumber];

        // dummy values for pixel for testing purposes
        //float pixel = rand();

        //write all wavelength absorbtion values for one(!) pixel to a vector
        inputVector[z] = pixel;
      }
      Eigen::VectorXf resultVector = SpectralUnmixingTestAlgorithm(EndmemberMatrix, inputVector);

      // write output
      for (int outputIdx = 0; outputIdx < GetNumberOfIndexedOutputs(); ++outputIdx)
      {
        auto output = GetOutput(outputIdx);
        mitk::ImageWriteAccessor writeOutput(output);
        float* writeBuffer = (float *)writeOutput.GetData();
        writeBuffer[x*yDim + y] = resultVector[outputIdx]; 
      }
    }
  }
  MITK_INFO << "GENERATING DATA...[DONE]";
}

void mitk::pa::SpectralUnmixingFilter::CheckPreConditions(unsigned int NumberOfInputImages)
{
  // Checking if number of Inputs == added wavelengths
  if (m_Wavelength.size() != NumberOfInputImages)
    mitkThrow() << "CHECK INPUTS! WAVELENGTHERROR";

  // Checking if number of wavelengths >= number of chromophores
  if (m_Chromophore.size() > m_Wavelength.size())
    mitkThrow() << "PRESS 'IGNORE' AND ADD MORE WAVELENGTHS!";

  MITK_INFO << "CHECK PRECONDITIONS ...[DONE]";
}

void mitk::pa::SpectralUnmixingFilter::InitializeOutputs()
{
  unsigned int numberOfInputs = GetNumberOfIndexedInputs();
  unsigned int numberOfOutputs = GetNumberOfIndexedOutputs();
  MITK_INFO << "Inputs: " << numberOfInputs << " Outputs: " << numberOfOutputs;

  //  Set dimensions (2) and pixel type (float) for output
  mitk::PixelType pixelType = mitk::MakeScalarPixelType<float>();
  const int NUMBER_OF_SPATIAL_DIMENSIONS = 2;
  auto* dimensions = new unsigned int[NUMBER_OF_SPATIAL_DIMENSIONS];
  for(unsigned int dimIdx=0; dimIdx<NUMBER_OF_SPATIAL_DIMENSIONS; dimIdx++)
  {
    dimensions[dimIdx] = GetInput()->GetDimensions()[dimIdx];
  }

  // Initialize numberOfOutput pictures with pixel type and dimensions
  for (unsigned int outputIdx = 0; outputIdx < numberOfOutputs; outputIdx++)
  {
    GetOutput(outputIdx)->Initialize(pixelType, NUMBER_OF_SPATIAL_DIMENSIONS, dimensions);
  }
}

//Matrix with #chromophores rows and #wavelengths columns
//so Matrix Element (i,j) contains the Absorbtion of chromophore i @ wavelength j
Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> mitk::pa::SpectralUnmixingFilter::AddEndmemberMatrix()
{
  unsigned int numberOfChromophores = m_Chromophore.size(); //rows
  unsigned int numberOfWavelengths = m_Wavelength.size(); //columns

  Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> EndmemberMatrix(numberOfChromophores, numberOfWavelengths);

  //loop over i rows (Chromophores)
  for(unsigned int i = 0; i < numberOfChromophores; ++i)
  {
    //loop over j columns (Wavelengths)
    for (unsigned int j = 0; j < numberOfWavelengths; ++j)
    {
      //writes @ Matrix element (i,j) the absorbtion wavelength of the propertycalculator.cpp
      EndmemberMatrix(i,j)= m_PropertyCalculator->GetAbsorptionForWavelength(
      static_cast<mitk::pa::PropertyCalculator::MapType>(m_Chromophore[i]), m_Wavelength[j]);
      /* Test to see what gets written in the Matrix:
      MITK_INFO << "map type: " << static_cast<mitk::pa::PropertyCalculator::MapType>(m_Chromophore[i]);
      MITK_INFO << "wavelength: " << m_Wavelength[j];
      MITK_INFO << "Matrixelement: (" << i << ", " << j << ") Absorbtion: " << EndmemberMatrix(i, j);/**/
      
      if (EndmemberMatrix(i, j) == 0)
      {
        m_Wavelength.clear();
        mitkThrow() << "WAVELENGTH "<< m_Wavelength[j] << "nm NOT SUPPORTED!";
      }
    }
  }
  MITK_INFO << "GENERATING ENMEMBERMATRIX [DONE]!";
  return EndmemberMatrix;
}

Eigen::VectorXf mitk::pa::SpectralUnmixingFilter::SpectralUnmixingTestAlgorithm(
  Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> EndmemberMatrix, Eigen::VectorXf inputVector)
{
  //test "solver" (input = output)
  Eigen::VectorXf resultVector = inputVector;
  return resultVector;
    
  //llt solver
  //Eigen::VectorXf resultVector = EndmemberMatrix.llt().solve(inputVector);
  //return resultVector;

  //householderqr solver
  //Eigen::VectorXf resultVector =EndmemberMatrix.householderQr().solve(inputVector);
  //return resultVector;

  //test other solvers https://eigen.tuxfamily.org/dox/group__TutorialLinearAlgebra.html
  // define treshold for relativ error and set value to eg. -1 ;)
  /*double relative_error = (EndmemberMatrix*inputVector - resultVector).norm() / resultVector.norm(); // norm() is L2 norm
  MITK_INFO << relative_error;/**/
}
