/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkPASpectralUnmixingFilterBase.h"

// Includes for AddEnmemberMatrix
#include "mitkPAPropertyCalculator.h"
#include <eigen3/Eigen/Dense>

// ImageAccessor
#include <mitkImageReadAccessor.h>
#include <mitkImageWriteAccessor.h>

mitk::pa::SpectralUnmixingFilterBase::SpectralUnmixingFilterBase()
{
  m_PropertyCalculatorEigen = mitk::pa::PropertyCalculator::New();
}

mitk::pa::SpectralUnmixingFilterBase::~SpectralUnmixingFilterBase()
{

}

void mitk::pa::SpectralUnmixingFilterBase::AddOutputs(unsigned int outputs)
{
  if (outputs == 0)
    mitkThrow() << "ADD OUTPUTS HAS TO BE LARGER THEN ZERO!";
  this->SetNumberOfIndexedOutputs(outputs);
  for (unsigned int i = 0; i<GetNumberOfIndexedOutputs(); i++)
    this->SetNthOutput(i, mitk::Image::New());
}

void mitk::pa::SpectralUnmixingFilterBase::AddWavelength(int wavelength)
{
  m_Wavelength.push_back(wavelength);
}

void mitk::pa::SpectralUnmixingFilterBase::AddChromophore(mitk::pa::PropertyCalculator::ChromophoreType chromophore)
{
  m_Chromophore.push_back(chromophore);
}

void mitk::pa::SpectralUnmixingFilterBase::Verbose(bool verbose)
{
  m_Verbose = verbose;
}

void mitk::pa::SpectralUnmixingFilterBase::RelativeError(bool relativeError)
{
  m_RelativeError = relativeError;
}

void mitk::pa::SpectralUnmixingFilterBase::AddRelativeErrorSettings(int value)
{
  m_RelativeErrorSettings.push_back(value);
}

void mitk::pa::SpectralUnmixingFilterBase::GenerateData()
{
  MITK_INFO(m_Verbose) << "GENERATING DATA..";

  mitk::Image::Pointer input = GetInput(0);
  CheckPreConditions(input);

  unsigned int xDim = input->GetDimensions()[0];
  unsigned int yDim = input->GetDimensions()[1];
  unsigned int numberOfInputImages = input->GetDimensions()[2];

  MITK_INFO(m_Verbose) << "x dimension: " << xDim;
  MITK_INFO(m_Verbose) << "y dimension: " << yDim;
  MITK_INFO(m_Verbose) << "z dimension: " << numberOfInputImages;

  // Copy input image into array
  mitk::ImageReadAccessor readAccess(input);
  const float* inputDataArray = ((const float*)readAccess.GetData());

  unsigned int sequenceSize = m_Wavelength.size();
  unsigned int totalNumberOfSequences = numberOfInputImages / sequenceSize;
  MITK_INFO(m_Verbose) << "TotalNumberOfSequences: " << totalNumberOfSequences;

  InitializeOutputs(totalNumberOfSequences);

  auto endmemberMatrix = CalculateEndmemberMatrix(m_Chromophore, m_Wavelength);

  // test to see pixel values @ txt file
  myfile.open("SimplexNormalisation.txt");

  unsigned int outputCounter = GetNumberOfIndexedOutputs();
  std::vector<float*> writteBufferVector;
  for (unsigned int i = 0; i < outputCounter; ++i)
  {
    auto output = GetOutput(i);
    mitk::ImageWriteAccessor writeOutput(output);
    float* writeBuffer = (float *)writeOutput.GetData();
    writteBufferVector.push_back(writeBuffer);
  }

  if (m_RelativeError == true)
  {
    // -1 because rel error is output[IndexedOutputs() - 1] and loop over chromophore outputs has to end at [IndexedOutputs() - 2]
    outputCounter -= 1;
  }

  for (unsigned int sequenceCounter = 0; sequenceCounter < totalNumberOfSequences; ++sequenceCounter)
  {
    MITK_INFO(m_Verbose) << "SequenceCounter: " << sequenceCounter;
    //loop over every pixel in XY-plane
    for (unsigned int x = 0; x < xDim; x++)
    {
      for (unsigned int y = 0; y < yDim; y++)
      {
        Eigen::VectorXf inputVector(sequenceSize);
        for (unsigned int z = 0; z < sequenceSize; z++)
        {
          /**
          * 'sequenceCounter*sequenceSize' has to be added to 'z' to ensure that one accesses the
          * correct pixel, because the inputDataArray contains the information of all sequences and
          * not just the one of the current sequence.
          */
          unsigned int pixelNumber = (xDim*yDim*(z+sequenceCounter*sequenceSize)) + x * yDim + y;
          auto pixel = inputDataArray[pixelNumber];

          inputVector[z] = pixel;
        }
        Eigen::VectorXf resultVector = SpectralUnmixingAlgorithm(endmemberMatrix, inputVector);

        if (m_RelativeError == true)
        {
          float relativeError = CalculateRelativeError(endmemberMatrix, inputVector, resultVector);
          writteBufferVector[outputCounter][(xDim*yDim * sequenceCounter) + x * yDim + y] = relativeError;
        }

        for (unsigned int outputIdx = 0; outputIdx < outputCounter; ++outputIdx)
        {
          writteBufferVector[outputIdx][(xDim*yDim * sequenceCounter) + x * yDim + y] = resultVector[outputIdx];
        }
      }
    }
  }
  MITK_INFO(m_Verbose) << "GENERATING DATA...[DONE]";
  myfile.close();
}

void mitk::pa::SpectralUnmixingFilterBase::CheckPreConditions(mitk::Image::Pointer input)
{
  MITK_INFO(m_Verbose) << "CHECK PRECONDITIONS ...";

  if (m_Chromophore.size() == 0 || m_Wavelength.size() == 0)
    mitkThrow() << "NO WAVELENGHTS/CHROMOPHORES SELECTED!";

  if (m_Wavelength.size() < input->GetDimensions()[2])
    MITK_WARN(m_Verbose) << "NUMBER OF WAVELENGTHS < NUMBER OF INPUT IMAGES";

  if (m_Chromophore.size() > m_Wavelength.size())
    mitkThrow() << "ADD MORE WAVELENGTHS OR REMOVE ENDMEMBERS!";

  if (input->GetPixelType() != mitk::MakeScalarPixelType<float>())
    mitkThrow() << "PIXELTYPE ERROR! FLOAT REQUIRED";

  if ((m_Chromophore.size()+ m_RelativeError )!= GetNumberOfIndexedOutputs() || input->GetDimensions()[2] < GetNumberOfIndexedOutputs())
    mitkThrow() << "INDEX ERROR! NUMBER OF OUTPUTS DOESN'T FIT TO OTHER SETTIGNS!";

  MITK_INFO(m_Verbose) << "...[DONE]";
}

void mitk::pa::SpectralUnmixingFilterBase::InitializeOutputs(unsigned int totalNumberOfSequences)
{
  MITK_INFO(m_Verbose) << "Initialize Outputs ...";

  unsigned int numberOfInputs = GetNumberOfIndexedInputs();
  unsigned int numberOfOutputs = GetNumberOfIndexedOutputs();
  MITK_INFO(m_Verbose) << "Inputs: " << numberOfInputs << " Outputs: " << numberOfOutputs;

  mitk::PixelType pixelType = mitk::MakeScalarPixelType<float>();
  const int NUMBER_OF_SPATIAL_DIMENSIONS = 3;
  auto* dimensions = new unsigned int[NUMBER_OF_SPATIAL_DIMENSIONS];
  for (unsigned int dimIdx = 0; dimIdx < 2; dimIdx++)
    dimensions[dimIdx] = GetInput()->GetDimensions()[dimIdx];
  dimensions[2] = totalNumberOfSequences;

  for (unsigned int outputIdx = 0; outputIdx < numberOfOutputs; outputIdx++)
    GetOutput(outputIdx)->Initialize(pixelType, NUMBER_OF_SPATIAL_DIMENSIONS, dimensions);
  MITK_INFO(m_Verbose) << "...[DONE]";
}

Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> mitk::pa::SpectralUnmixingFilterBase::CalculateEndmemberMatrix(
  std::vector<mitk::pa::PropertyCalculator::ChromophoreType> m_Chromophore, std::vector<int> m_Wavelength)
{
  unsigned int numberOfChromophores = m_Chromophore.size(); //columns
  unsigned int numberOfWavelengths = m_Wavelength.size(); //rows
  Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> endmemberMatrixEigen(numberOfWavelengths, numberOfChromophores);

  for (unsigned int j = 0; j < numberOfChromophores; ++j)
  {
    for (unsigned int i = 0; i < numberOfWavelengths; ++i)
      endmemberMatrixEigen(i, j) = PropertyElement(m_Chromophore[j], m_Wavelength[i]);
  }
  MITK_INFO(m_Verbose) << "GENERATING ENMEMBERMATRIX [DONE]";
  return endmemberMatrixEigen;
}

float mitk::pa::SpectralUnmixingFilterBase::PropertyElement(mitk::pa::PropertyCalculator::ChromophoreType chromophore, int wavelength)
{
  if (chromophore == mitk::pa::PropertyCalculator::ChromophoreType::ONEENDMEMBER)
    return 1;
  else
  {
    float value = m_PropertyCalculatorEigen->GetAbsorptionForWavelength(chromophore, wavelength);
    if (value == 0)
      mitkThrow() << "WAVELENGTH " << wavelength << "nm NOT SUPPORTED!";
    else
      return value;
  }
}

float mitk::pa::SpectralUnmixingFilterBase::CalculateRelativeError(Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> endmemberMatrix,
  Eigen::VectorXf inputVector, Eigen::VectorXf resultVector)
{
  float relativeError = (endmemberMatrix*resultVector - inputVector).norm() / inputVector.norm();
  for (int i = 0; i < 2; ++i)
  {
    if (resultVector[i] < m_RelativeErrorSettings[i])
      return 0;
  }
  return relativeError;
}
