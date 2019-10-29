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
  this->SetNumberOfIndexedOutputs(2);
  this->SetNthOutput(0, mitk::Image::New());
  this->SetNthOutput(1, mitk::Image::New());

}

mitk::pa::SpectralUnmixingSO2::~SpectralUnmixingSO2()
{

}

void mitk::pa::SpectralUnmixingSO2::Verbose(bool verbose)
{
  m_Verbose = verbose;
}

void mitk::pa::SpectralUnmixingSO2::GenerateData()
{
  MITK_INFO(m_Verbose) << "GENERATING DATA..";

  // Get input image
  mitk::Image::Pointer inputHbO2 = GetInput(0);
  mitk::Image::Pointer inputHb = GetInput(1);

  CheckPreConditions(inputHbO2, inputHb);

  unsigned int xDim = inputHbO2->GetDimensions()[0];
  unsigned int yDim = inputHbO2->GetDimensions()[1];
  unsigned int zDim = inputHbO2->GetDimensions()[2];

  InitializeOutputs();

  mitk::ImageReadAccessor readAccessHbO2(inputHbO2);
  mitk::ImageReadAccessor readAccessHb(inputHb);

  const float* inputDataArrayHbO2 = ((const float*)readAccessHbO2.GetData());
  const float* inputDataArrayHb = ((const float*)readAccessHb.GetData());

  auto output = GetOutput(0);
  auto output1 = GetOutput(1);

  mitk::ImageWriteAccessor writeOutput(output);
  float* writeBuffer = (float *)writeOutput.GetData();

  mitk::ImageWriteAccessor writeOutput1(output1);
  float* writeBuffer1 = (float *)writeOutput1.GetData();

  for (unsigned int x = 0; x < xDim; x++)
  {
    for (unsigned int y = 0; y < yDim; y++)
    {
      for (unsigned int z = 0;z < zDim; z++)
      {
        unsigned int pixelNumber = (xDim*yDim * z) + x * yDim + y;
        float pixelHb = inputDataArrayHb[pixelNumber];
        float pixelHbO2 = inputDataArrayHbO2[pixelNumber];
        float resultSO2 = CalculateSO2(pixelHb, pixelHbO2);
        writeBuffer[(xDim*yDim * z) + x * yDim + y] = resultSO2;
        float resultTHb = CalculateTHb(pixelHb, pixelHbO2);
        writeBuffer1[(xDim*yDim * z) + x * yDim + y] = resultTHb;
      }
    }
  }
  MITK_INFO(m_Verbose) << "GENERATING DATA...[DONE]";
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

  if (inputHbO2->GetPixelType() != mitk::MakeScalarPixelType<float>())
    mitkThrow() << "PIXELTYPE ERROR! FLOAT REQUIRED";

  if (inputHb->GetPixelType() != mitk::MakeScalarPixelType<float>())
    mitkThrow() << "PIXELTYPE ERROR! FLOAT REQUIRED";

  MITK_INFO(m_Verbose) << "CHECK PRECONDITIONS ...[DONE]";
}

void mitk::pa::SpectralUnmixingSO2::InitializeOutputs()
{
  // UNUSED unsigned int numberOfInputs = GetNumberOfIndexedInputs();
  unsigned int numberOfOutputs = GetNumberOfIndexedOutputs();

  mitk::PixelType pixelType = mitk::MakeScalarPixelType<float>();
  const int NUMBER_OF_SPATIAL_DIMENSIONS = 3;
  auto* dimensions = new unsigned int[NUMBER_OF_SPATIAL_DIMENSIONS];
  for(unsigned int dimIdx=0; dimIdx<NUMBER_OF_SPATIAL_DIMENSIONS; dimIdx++)
  {
    dimensions[dimIdx] = GetInput()->GetDimensions()[dimIdx];
  }

  for (unsigned int outputIdx = 0; outputIdx < numberOfOutputs; outputIdx++)
  {
    GetOutput(outputIdx)->Initialize(pixelType, NUMBER_OF_SPATIAL_DIMENSIONS, dimensions);
  }
}

float mitk::pa::SpectralUnmixingSO2::CalculateSO2(float Hb, float HbO2)
{
  float result = HbO2 / (Hb + HbO2);

  if (result != result)
  {
    MITK_WARN(m_Verbose) << "SO2 VALUE NAN! WILL BE SET TO ZERO!";
    return 0;
  }
  else
  {
    if (SO2ValueNotSiginificant(Hb, HbO2, result))
    {
      return 0;
    }
    else return result;
  }
}

float mitk::pa::SpectralUnmixingSO2::CalculateTHb(float Hb, float HbO2)
{
  float result = (Hb + HbO2);

  if (result != result)
  {
    MITK_WARN(m_Verbose) << "SO2 VALUE NAN! WILL BE SET TO ZERO!";
    return 0;
  }
  else
  {
    return result;
  }
}

void mitk::pa::SpectralUnmixingSO2::AddSO2Settings(int value)
{
  m_SO2Settings.push_back(value);
}

bool mitk::pa::SpectralUnmixingSO2::SO2ValueNotSiginificant(float Hb, float HbO2, float result)
{
  std::vector<float> significant;
  significant.push_back(HbO2);
  significant.push_back(Hb);
  significant.push_back(HbO2 + Hb);
  significant.push_back(100*(result));

  for (unsigned int i = 0; i < m_SO2Settings.size(); ++i)
  {
    if (m_SO2Settings[i] != 0 && m_SO2Settings[i] > significant[i] && (std::abs(m_SO2Settings[i] - significant[i]) > 1e-7))
    {
      return true;
    }
  }
  return false;
}
