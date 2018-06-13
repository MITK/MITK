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

#include "mitkPASpectralUnmixingSO2.h"

// ImageAccessor
#include <mitkImageReadAccessor.h>
#include <mitkImageWriteAccessor.h>

mitk::pa::SpectralUnmixingSO2::SpectralUnmixingSO2()
{
  this->SetNumberOfIndexedInputs(2);
  this->SetNumberOfIndexedOutputs(1);
  this->SetNthOutput(0, mitk::Image::New());
}

mitk::pa::SpectralUnmixingSO2::~SpectralUnmixingSO2()
{

}

void mitk::pa::SpectralUnmixingSO2::GenerateData()
{
  MITK_INFO << "GENERATING DATA..";

  // Get input image
  mitk::Image::Pointer inputHbO2 = GetInput(0);
  mitk::Image::Pointer inputHb = GetInput(1);

  CheckPreConditions(inputHbO2, inputHb);

  unsigned int xDim = inputHbO2->GetDimensions()[0];
  unsigned int yDim = inputHbO2->GetDimensions()[1];
  unsigned int zDim = inputHbO2->GetDimensions()[2];
  unsigned int size = xDim * yDim*zDim;

  MITK_INFO << "x dimension: " << xDim;
  MITK_INFO << "y dimension: " << yDim;
  MITK_INFO << "z dimension: " << zDim;

  InitializeOutputs();

  // Copy input image into array
  mitk::ImageReadAccessor readAccessHbO2(inputHbO2);
  mitk::ImageReadAccessor readAccessHb(inputHb);

  const float* inputDataArrayHbO2 = ((const float*)readAccessHbO2.GetData());
  const float* inputDataArrayHb = ((const float*)readAccessHb.GetData());

  for (unsigned int x = 0; x < xDim; x++)
  {
    for (unsigned int y = 0; y < yDim; y++)
    {
      for (unsigned int z = 0;z < zDim; z++)
      {
        // Calculate SO2 and write to output
        unsigned int pixelNumber = (xDim*yDim * z) + x * yDim + y;
        float pixelHb = inputDataArrayHb[pixelNumber];
        float pixelHbO2 = inputDataArrayHbO2[pixelNumber];
        float resultSO2 = calculateSO2(pixelHb, pixelHbO2);
        auto output = GetOutput(0);
        mitk::ImageWriteAccessor writeOutput(output);
        float* writeBuffer = (float *)writeOutput.GetData();
        writeBuffer[(xDim*yDim * z) + x * yDim + y] = resultSO2;
      }
    }
  }
  MITK_INFO << "GENERATING DATA...[DONE]";
}

void mitk::pa::SpectralUnmixingSO2::CheckPreConditions(mitk::Image::Pointer inputHbO2, mitk::Image::Pointer inputHb)
{
  unsigned int xDimHb = inputHb->GetDimensions()[0];
  unsigned int yDimHb = inputHb->GetDimensions()[1];
  unsigned int zDimHb = inputHb->GetDimensions()[2];

  unsigned int xDimHbO2 = inputHbO2->GetDimensions()[0];
  unsigned int yDimHbO2 = inputHbO2->GetDimensions()[1];
  unsigned int zDimHbO2 = inputHbO2->GetDimensions()[2];

  if (xDimHb != xDimHbO2 || yDimHb != yDimHbO2 || zDimHb != zDimHbO2)
    mitkThrow() << "DIMENTIONALITY ERROR!";

  MITK_INFO << "CHECK PRECONDITIONS ...[DONE]";
}

void mitk::pa::SpectralUnmixingSO2::InitializeOutputs()
{
  unsigned int numberOfInputs = GetNumberOfIndexedInputs();
  unsigned int numberOfOutputs = GetNumberOfIndexedOutputs();

  //  Set dimensions (3) and pixel type (float) for output
  mitk::PixelType pixelType = mitk::MakeScalarPixelType<float>();
  const int NUMBER_OF_SPATIAL_DIMENSIONS = 3;
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

float mitk::pa::SpectralUnmixingSO2::calculateSO2(float Hb, float HbO2)
{
  float MinHb = m_SO2Settings[0];
  float MinHbO2 = m_SO2Settings[1];
  float MinSum = m_SO2Settings[2];
  float MinSO2 = m_SO2Settings[3];
  float result = HbO2 / (Hb + HbO2);

  if (MinHb != 0 && MinHb > Hb)
    return 0;
  else if (MinHbO2 != 0 && MinHbO2 > HbO2)
    return 0;
  else if (MinSum != 0 && MinSum > Hb + HbO2)
    return 0;
  else if (MinSO2 != 0 && 100*result > MinSO2)
    return result;
  else if (MinSO2 != 0 && result < MinSO2)
    return 0;
  else
    return result;
}

void mitk::pa::SpectralUnmixingSO2::AddSO2Settings(float value)
{
  m_SO2Settings.push_back(value);
}
