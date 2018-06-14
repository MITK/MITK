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

#include <chrono>

mitk::pa::SpectralUnmixingFilterBase::SpectralUnmixingFilterBase()
{
  this->SetNumberOfIndexedOutputs(3);// find solution --> 4 is max outputs

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

void mitk::pa::SpectralUnmixingFilterBase::AddWeight(int Weight)
{
  m_Weight.push_back(Weight);
  MITK_INFO << "Weight: " << m_Weight[m_Weight.size()-1];
}

void mitk::pa::SpectralUnmixingFilterBase::GenerateData()
{
  MITK_INFO << "GENERATING DATA..";

  // Get input image
  mitk::Image::Pointer input = GetInput(0);

  unsigned int xDim = input->GetDimensions()[0];
  unsigned int yDim = input->GetDimensions()[1];
  unsigned int NumberOfInputImages = input->GetDimensions()[2];
  unsigned int size = xDim * yDim * NumberOfInputImages;

  MITK_INFO << "x dimension: " << xDim;
  MITK_INFO << "y dimension: " << yDim;
  MITK_INFO << "z dimension: " << NumberOfInputImages;

  unsigned int sequenceSize = m_Wavelength.size();
  unsigned int TotalNumberOfSequences = NumberOfInputImages / sequenceSize;
  MITK_INFO << "TotalNumberOfSequences: " << TotalNumberOfSequences;

  InitializeOutputs(TotalNumberOfSequences);
  
  auto EndmemberMatrix = CalculateEndmemberMatrix(m_Chromophore, m_Wavelength); //Eigen Endmember Matrix
  
  // Copy input image into array
  mitk::ImageReadAccessor readAccess(input);
  const float* inputDataArray = ((const float*)readAccess.GetData());

  CheckPreConditions(size, NumberOfInputImages, inputDataArray);

  // test to see pixel values @ txt file
  //ofstream myfile;
  //myfile.open("PASpectralUnmixingPixelValues.txt");
  std::chrono::steady_clock::time_point _start(std::chrono::steady_clock::now());

  myfile.open("SimplexNormalisation.txt");

  for (unsigned int SequenceCounter = 0; SequenceCounter < TotalNumberOfSequences;++SequenceCounter)
  {

    MITK_INFO << "SequenceCounter: " << SequenceCounter;
    //loop over every pixel @ x,y plane
    for (unsigned int x = 0; x < xDim; x++)
    {
      for (unsigned int y = 0; y < yDim; y++)
      {
        Eigen::VectorXf inputVector(sequenceSize);
        for (unsigned int z = 0; z < sequenceSize; z++)
        {
          // Get pixel value of pixel x,y @ wavelength z
          unsigned int pixelNumber = (xDim*yDim*(z+SequenceCounter*sequenceSize)) + x * yDim + y;
          auto pixel = inputDataArray[pixelNumber];

          //write all wavelength absorbtion values for one(!) pixel to a vector
          inputVector[z] = pixel;
        }
        Eigen::VectorXf resultVector = SpectralUnmixingAlgorithm(EndmemberMatrix, inputVector);

        // write output
        for (int outputIdx = 0; outputIdx < GetNumberOfIndexedOutputs(); ++outputIdx)
        {
          auto output = GetOutput(outputIdx);
          mitk::ImageWriteAccessor writeOutput(output);
          float* writeBuffer = (float *)writeOutput.GetData();
          writeBuffer[(xDim*yDim * SequenceCounter) + x * yDim + y] = resultVector[outputIdx];
        }
        //myfile << "Input Pixel(x,y): " << x << "," << y << "\n" << inputVector << "\n";
        //myfile << "Result: "  << "\n HbO2: " << resultVector[0] << "\n Hb: " << resultVector[1] << "\n Mel: "
        //  << resultVector[2] << "Result vector size" << resultVector.size() << "\n";
      }
    }
  }
  std::chrono::steady_clock::time_point _end(std::chrono::steady_clock::now());
  std::chrono::steady_clock::time_point _test(std::chrono::steady_clock::now());
  MITK_INFO << "Chronotets: " << std::chrono::duration_cast<std::chrono::duration<double>>(_test - _start).count() << "s";

  MITK_INFO << "Chrono: " << std::chrono::duration_cast<std::chrono::duration<double>>(_end - _start).count() << "s";
  MITK_INFO << "GENERATING DATA...[DONE]";
  myfile.close();
}

void mitk::pa::SpectralUnmixingFilterBase::CheckPreConditions(unsigned int size, unsigned int NumberOfInputImages, const float* inputDataArray)
{
  // Checking if number of Inputs == added wavelengths
  if (m_Wavelength.size() < NumberOfInputImages)
    MITK_WARN << "NUMBER OF WAVELENGTHS < NUMBER OF INPUT IMAGES";

  if (m_Wavelength.size() > NumberOfInputImages)
    mitkThrow() << "ERROR! REMOVE WAVELENGTHS!";

  // Checking if number of wavelengths >= number of chromophores
  if (m_Chromophore.size() > m_Wavelength.size())
    mitkThrow() << "PRESS 'IGNORE' AND ADD MORE WAVELENGTHS!";

  // Checking if pixel type is float
  int maxPixel = size;
  for (int i = 0; i < maxPixel; ++i)
  {
    if (typeid(inputDataArray[i]).name() != typeid(float).name())
      mitkThrow() << "PIXELTYPE ERROR! FLOAT 32 REQUIRED";

    else continue;
  }
  MITK_INFO << "CHECK PRECONDITIONS ...[DONE]";
}

void mitk::pa::SpectralUnmixingFilterBase::InitializeOutputs(unsigned int TotalNumberOfSequences)
{
  unsigned int numberOfInputs = GetNumberOfIndexedInputs();
  unsigned int numberOfOutputs = GetNumberOfIndexedOutputs();
  MITK_INFO << "Inputs: " << numberOfInputs << " Outputs: " << numberOfOutputs;

  //  Set dimensions (3) and pixel type (float) for output
  mitk::PixelType pixelType = mitk::MakeScalarPixelType<float>();
  const int NUMBER_OF_SPATIAL_DIMENSIONS = 3;
  auto* dimensions = new unsigned int[NUMBER_OF_SPATIAL_DIMENSIONS];
  for (unsigned int dimIdx = 0; dimIdx < 2; dimIdx++)
  {
    dimensions[dimIdx] = GetInput()->GetDimensions()[dimIdx];
  }
  dimensions[2] = TotalNumberOfSequences;

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
  return EndmemberMatrixEigen;
}

float mitk::pa::SpectralUnmixingFilterBase::propertyElement(mitk::pa::PropertyCalculator::ChromophoreType Chromophore, int Wavelength)
{
  float value = m_PropertyCalculatorEigen->GetAbsorptionForWavelength(Chromophore, Wavelength);
  if (value == 0)
    mitkThrow() << "WAVELENGTH " << Wavelength << "nm NOT SUPPORTED!";
  return value;
}
