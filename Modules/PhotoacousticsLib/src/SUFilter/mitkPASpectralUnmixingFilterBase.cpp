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

#include "mitkPASpectralUnmixingFilterBase.h"

// Includes for AddEnmemberMatrix
#include "mitkPAPropertyCalculator.h"
#include <eigen3/Eigen/Dense>

// ImageAccessor
#include <mitkImageReadAccessor.h>
#include <mitkImageWriteAccessor.h>

mitk::pa::SpectralUnmixingFilterBase::SpectralUnmixingFilterBase()
{
  this->SetNumberOfIndexedOutputs(4);// find solution

  for (unsigned int i = 0; i<GetNumberOfIndexedOutputs(); i++)
  {
    this->SetNthOutput(i, mitk::Image::New());
  }

  m_PropertyCalculatorEigen = mitk::pa::PropertyCalculator::New();
}

mitk::pa::SpectralUnmixingFilterBase::~SpectralUnmixingFilterBase()
{

}

void mitk::pa::SpectralUnmixingFilterBase::AddWavelength(int wavelength)
{
  m_Wavelength.push_back(wavelength);
}

void mitk::pa::SpectralUnmixingFilterBase::AddChromophore(mitk::pa::PropertyCalculator::ChromophoreType chromophore)
{
  m_Chromophore.push_back(chromophore);
}

void mitk::pa::SpectralUnmixingFilterBase::GenerateData()
{
  MITK_INFO << "GENERATING DATA..";

  // Get input image
  mitk::Image::Pointer input = GetInput(0);

  unsigned int xDim = input->GetDimensions()[0];
  unsigned int yDim = input->GetDimensions()[1];
  unsigned int zDim = input->GetDimensions()[2];
  unsigned int size = xDim * yDim * zDim;

  MITK_INFO << "x dimension: " << xDim;
  MITK_INFO << "y dimension: " << yDim;
  MITK_INFO << "z dimension: " << zDim;

  InitializeOutputs();
  
  auto EndmemberMatrix = CalculateEndmemberMatrix(m_Chromophore, m_Wavelength); //Eigen Endmember Matrix
  
  // Copy input image into array
  mitk::ImageReadAccessor readAccess(input);

  const float* inputDataArray = ((const float*)readAccess.GetData());

  CheckPreConditions(size, zDim, inputDataArray);

  /* READ OUT INPUTARRAY
  MITK_INFO << "Info Array:";
  int numberOfPixels= 6;
  for (int i=0; i< numberOfPixels; ++i)
    MITK_INFO << inputDataArray[i];/**/

  // test to see pixel values @ txt file
  ofstream myfile;
  myfile.open("PASpectralUnmixingPixelValues.txt");

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

        //MITK_INFO << "Pixel_values: " << pixel;

        // dummy values for pixel for testing purposes
        //float pixel = rand();

        //write all wavelength absorbtion values for one(!) pixel to a vector
        inputVector[z] = pixel;
      }
      Eigen::VectorXf resultVector = SpectralUnmixingAlgorithm(EndmemberMatrix, inputVector);
      //Eigen::VectorXf resultVector = SpectralUnmixingAlgorithm(m_Chromophore, inputVector);


      // write output
      for (int outputIdx = 0; outputIdx < GetNumberOfIndexedOutputs(); ++outputIdx)
      {
        auto output = GetOutput(outputIdx);
        mitk::ImageWriteAccessor writeOutput(output);
        float* writeBuffer = (float *)writeOutput.GetData();
        writeBuffer[x*yDim + y] = resultVector[outputIdx]; 
      }
      myfile << "Input Pixel(x,y): " << x << "," << y << "\n" << inputVector << "\n";
      myfile << "Result: "  << "\n HbO2: " << resultVector[0] << "\n Hb: " << resultVector[1] << "\n Mel: "
        << resultVector[2] << "Result vector size" << resultVector.size() << "\n";
    }
  }
  MITK_INFO << "GENERATING DATA...[DONE]";
  myfile.close();
}

void mitk::pa::SpectralUnmixingFilterBase::CheckPreConditions(unsigned int size, unsigned int NumberOfInputImages, const float* inputDataArray)
{
  // Checking if number of Inputs == added wavelengths
  if (m_Wavelength.size() != NumberOfInputImages)
    mitkThrow() << "CHECK INPUTS! WAVELENGTHERROR";

  // Checking if number of wavelengths >= number of chromophores
  if (m_Chromophore.size() > m_Wavelength.size())
    mitkThrow() << "PRESS 'IGNORE' AND ADD MORE WAVELENGTHS!";

  // Checking if pixel type is float
  int maxPixel = size;
  for (int i = 0; i < maxPixel; ++i)
  {
    if (typeid(inputDataArray[i]).name() != typeid(float).name())
    {
      mitkThrow() << "PIXELTYPE ERROR! FLOAT 32 REQUIRED";
    }
    else continue;
  }

  MITK_INFO << "CHECK PRECONDITIONS ...[DONE]";
}

void mitk::pa::SpectralUnmixingFilterBase::InitializeOutputs()
{
  unsigned int numberOfInputs = GetNumberOfIndexedInputs();
  unsigned int numberOfOutputs = GetNumberOfIndexedOutputs();
  //MITK_INFO << "Inputs: " << numberOfInputs << " Outputs: " << numberOfOutputs;

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


//Matrix with #chromophores colums and #wavelengths rows
//so Matrix Element (i,j) contains the Absorbtion of chromophore j @ wavelength i
Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> mitk::pa::SpectralUnmixingFilterBase::CalculateEndmemberMatrix(
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
      {
        EndmemberMatrixEigen(i, j) = propertyElement(m_Chromophore[j], m_Wavelength[i]);
        MITK_INFO << "MELANIN " << EndmemberMatrixEigen(i, j);
      }
    }
    else if (m_Chromophore[j] == mitk::pa::PropertyCalculator::ChromophoreType::ONEENDMEMBER)
    {
      for (unsigned int i = 0; i < numberOfWavelengths; ++i)
        EndmemberMatrixEigen(i, j) = 1;
    }
    else
      mitkThrow() << "404 CHROMOPHORE NOT FOUND!";
  }
  MITK_INFO << "GENERATING ENMEMBERMATRIX [DONE]!";
  MITK_INFO << "endmember matrix: " << EndmemberMatrixEigen;
  return EndmemberMatrixEigen;
}

float mitk::pa::SpectralUnmixingFilterBase::propertyElement(mitk::pa::PropertyCalculator::ChromophoreType Chromophore, int Wavelength)
{
  float value = m_PropertyCalculatorEigen->GetAbsorptionForWavelength(Chromophore, Wavelength);
  if (value == 0)
    mitkThrow() << "WAVELENGTH " << Wavelength << "nm NOT SUPPORTED!";
  return value;
}
