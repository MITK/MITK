/*===================================================================
mitkPhotoacousticBeamformingFilter
The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "mitkProperties.h"
#include "mitkImageReadAccessor.h"
#include <algorithm>
#include <itkImageIOBase.h>
#include <chrono>
#include <thread>
#include <itkImageIOBase.h>
#include "mitkImageCast.h"
#include "mitkBeamformingUtils.h"

mitk::BeamformingUtils::BeamformingUtils()
{
}

mitk::BeamformingUtils::~BeamformingUtils()
{
}

float* mitk::BeamformingUtils::VonHannFunction(int samples)
{
  float* ApodWindow = new float[samples];

  for (int n = 0; n < samples; ++n)
  {
    ApodWindow[n] = (1 - cos(2 * itk::Math::pi * n / (samples - 1))) / 2;
  }

  return ApodWindow;
}

float* mitk::BeamformingUtils::HammFunction(int samples)
{
  float* ApodWindow = new float[samples];

  for (int n = 0; n < samples; ++n)
  {
    ApodWindow[n] = 0.54 - 0.46*cos(2 * itk::Math::pi*n / (samples - 1));
  }

  return ApodWindow;
}

float* mitk::BeamformingUtils::BoxFunction(int samples)
{
  float* ApodWindow = new float[samples];

  for (int n = 0; n < samples; ++n)
  {
    ApodWindow[n] = 1;
  }

  return ApodWindow;
}

void mitk::BeamformingUtils::DASQuadraticLine(
  float* input, float* output, float inputDim[2], float outputDim[2],
  const short& line, const mitk::BeamformingSettings::Pointer config)
{
  const float* apodisation = config->GetApodizationFunction();
  const short apodArraySize = config->GetApodizationArraySize();
  float& inputS = inputDim[1];
  float& inputL = inputDim[0];

  float& outputS = outputDim[1];
  float& outputL = outputDim[0];

  short AddSample = 0;
  short maxLine = 0;
  short minLine = 0;
  float delayMultiplicator = 0;
  float l_i = 0;
  float s_i = 0;

  float part = 0;
  float tan_phi = std::tan(config->GetAngle() / 360 * 2 * itk::Math::pi);
  float part_multiplicator = tan_phi * config->GetTimeSpacing() * config->GetSpeedOfSound() /
    config->GetPitchInMeters() * inputL / config->GetTransducerElements();
  float apod_mult = 1;

  short usedLines = (maxLine - minLine);

  float percentOfImageReconstructed = (float)(config->GetReconstructionDepth()) /
    (float)(inputS * config->GetSpeedOfSound() * config->GetTimeSpacing() / (float)(2 - (int)config->GetIsPhotoacousticImage()));
  percentOfImageReconstructed = percentOfImageReconstructed <= 1 ? percentOfImageReconstructed : 1;

  l_i = (float)line / outputL * inputL;

  for (short sample = 0; sample < outputS; ++sample)
  {
    s_i = (float)sample / outputS * inputS / (float)(2 - (int)config->GetIsPhotoacousticImage()) * percentOfImageReconstructed;

    part = part_multiplicator*s_i;

    if (part < 1)
      part = 1;

    maxLine = (short)std::min((l_i + part) + 1, inputL);
    minLine = (short)std::max((l_i - part), 0.0f);
    usedLines = (maxLine - minLine);

    apod_mult = (float)apodArraySize / (float)usedLines;

    delayMultiplicator = pow((1 / (config->GetTimeSpacing()*config->GetSpeedOfSound()) *
      (config->GetPitchInMeters()*config->GetTransducerElements()) / inputL), 2) / s_i / 2;

    for (short l_s = minLine; l_s < maxLine; ++l_s)
    {
      AddSample = delayMultiplicator * pow((l_s - l_i), 2) + s_i + (1 - config->GetIsPhotoacousticImage())*s_i;
      if (AddSample < inputS && AddSample >= 0)
        output[sample*(short)outputL + line] += input[l_s + AddSample*(short)inputL] *
        apodisation[(short)((l_s - minLine)*apod_mult)];
      else
        --usedLines;
    }
    output[sample*(short)outputL + line] = output[sample*(short)outputL + line] / usedLines;
  }
}

void mitk::BeamformingUtils::DASSphericalLine(
  float* input, float* output, float inputDim[2], float outputDim[2],
  const short& line, const mitk::BeamformingSettings::Pointer config)
{
  const float* apodisation = config->GetApodizationFunction();
  const short apodArraySize = config->GetApodizationArraySize();

  float& inputS = inputDim[1];
  float& inputL = inputDim[0];

  float& outputS = outputDim[1];
  float& outputL = outputDim[0];

  short AddSample = 0;
  short maxLine = 0;
  short minLine = 0;
  float l_i = 0;
  float s_i = 0;

  float part = 0.07 * inputL;
  float tan_phi = std::tan(config->GetAngle() / 360 * 2 * itk::Math::pi);
  float part_multiplicator = tan_phi * config->GetTimeSpacing() *
    config->GetSpeedOfSound() / config->GetPitchInMeters() * inputL / (float)config->GetTransducerElements();
  float apod_mult = 1;

  short usedLines = (maxLine - minLine);

  float percentOfImageReconstructed = (float)(config->GetReconstructionDepth()) /
    (float)(inputS * config->GetSpeedOfSound() * config->GetTimeSpacing() / (float)(2 - (int)config->GetIsPhotoacousticImage()));
  percentOfImageReconstructed = percentOfImageReconstructed <= 1 ? percentOfImageReconstructed : 1;

  l_i = (float)line / outputL * inputL;

  for (short sample = 0; sample < outputS; ++sample)
  {
    s_i = (float)sample / outputS * inputS / (float)(2 - (int)config->GetIsPhotoacousticImage()) * percentOfImageReconstructed;

    part = part_multiplicator*s_i;

    if (part < 1)
      part = 1;

    maxLine = (short)std::min((l_i + part) + 1, inputL);
    minLine = (short)std::max((l_i - part), 0.0f);
    usedLines = (maxLine - minLine);

    apod_mult = (float)apodArraySize / (float)usedLines;

    for (short l_s = minLine; l_s < maxLine; ++l_s)
    {
      AddSample = (int)sqrt(
        pow(s_i, 2)
        +
        pow((1 / (config->GetTimeSpacing()*config->GetSpeedOfSound()) *
        (((float)l_s - l_i)*config->GetPitchInMeters()*(float)config->GetTransducerElements()) / inputL), 2)
      ) + (1 - config->GetIsPhotoacousticImage())*s_i;
      if (AddSample < inputS && AddSample >= 0)
        output[sample*(short)outputL + line] += input[l_s + AddSample*(short)inputL] *
        apodisation[(short)((l_s - minLine)*apod_mult)];
      else
        --usedLines;
    }
    output[sample*(short)outputL + line] = output[sample*(short)outputL + line] / usedLines;
  }
}

void mitk::BeamformingUtils::DMASQuadraticLine(
  float* input, float* output, float inputDim[2], float outputDim[2],
  const short& line, const mitk::BeamformingSettings::Pointer config)
{
  const float* apodisation = config->GetApodizationFunction();
  const short apodArraySize = config->GetApodizationArraySize();

  float& inputS = inputDim[1];
  float& inputL = inputDim[0];

  float& outputS = outputDim[1];
  float& outputL = outputDim[0];

  short maxLine = 0;
  short minLine = 0;
  float delayMultiplicator = 0;
  float l_i = 0;
  float s_i = 0;

  float part = 0.07 * inputL;
  float tan_phi = std::tan(config->GetAngle() / 360 * 2 * itk::Math::pi);
  float part_multiplicator = tan_phi * config->GetTimeSpacing() *
    config->GetSpeedOfSound() / config->GetPitchInMeters() * inputL / (float)config->GetTransducerElements();
  float apod_mult = 1;

  float mult = 0;
  short usedLines = (maxLine - minLine);

  float percentOfImageReconstructed = (float)(config->GetReconstructionDepth()) /
    (float)(inputS * config->GetSpeedOfSound() * config->GetTimeSpacing() / (float)(2 - (int)config->GetIsPhotoacousticImage()));
  percentOfImageReconstructed = percentOfImageReconstructed <= 1 ? percentOfImageReconstructed : 1;

  l_i = (float)line / outputL * inputL;

  for (short sample = 0; sample < outputS; ++sample)
  {
    s_i = (float)sample / outputS * inputS / (float)(2 - (int)config->GetIsPhotoacousticImage()) * percentOfImageReconstructed;

    part = part_multiplicator*s_i;

    if (part < 1)
      part = 1;

    maxLine = (short)std::min((l_i + part) + 1, inputL);
    minLine = (short)std::max((l_i - part), 0.0f);
    usedLines = (maxLine - minLine);

    apod_mult = (float)apodArraySize / (float)usedLines;

    delayMultiplicator = pow((1 / (config->GetTimeSpacing()*config->GetSpeedOfSound()) *
      (config->GetPitchInMeters()*config->GetTransducerElements()) / inputL), 2) / s_i / 2;

    //calculate the AddSamples beforehand to save some time
    short* AddSample = new short[maxLine - minLine];
    for (short l_s = 0; l_s < maxLine - minLine; ++l_s)
    {
      AddSample[l_s] = (short)(delayMultiplicator * pow((minLine + l_s - l_i), 2) + s_i) +
        (1 - config->GetIsPhotoacousticImage())*s_i;
    }

    float s_1 = 0;
    float s_2 = 0;

    for (short l_s1 = minLine; l_s1 < maxLine - 1; ++l_s1)
    {
      if (AddSample[l_s1 - minLine] < inputS && AddSample[l_s1 - minLine] >= 0)
      {
        for (short l_s2 = l_s1 + 1; l_s2 < maxLine; ++l_s2)
        {
          if (AddSample[l_s2 - minLine] < inputS && AddSample[l_s2 - minLine] >= 0)
          {
            s_2 = input[l_s2 + AddSample[l_s2 - minLine] * (short)inputL];
            s_1 = input[l_s1 + AddSample[l_s1 - minLine] * (short)inputL];

            mult = s_2 * apodisation[(int)((l_s2 - minLine)*apod_mult)] * s_1 * apodisation[(int)((l_s1 - minLine)*apod_mult)];
            output[sample*(short)outputL + line] += sqrt(fabs(mult)) * ((mult > 0) - (mult < 0));
          }
        }
      }
      else
        --usedLines;
    }

    output[sample*(short)outputL + line] = output[sample*(short)outputL + line] / (float)(pow(usedLines, 2) - (usedLines - 1));

    delete[] AddSample;
  }
}

void mitk::BeamformingUtils::DMASSphericalLine(
  float* input, float* output, float inputDim[2], float outputDim[2],
  const short& line, const mitk::BeamformingSettings::Pointer config)
{
  const float* apodisation = config->GetApodizationFunction();
  const short apodArraySize = config->GetApodizationArraySize();

  float& inputS = inputDim[1];
  float& inputL = inputDim[0];

  float& outputS = outputDim[1];
  float& outputL = outputDim[0];

  short maxLine = 0;
  short minLine = 0;
  float l_i = 0;
  float s_i = 0;

  float part = 0.07 * inputL;
  float tan_phi = std::tan(config->GetAngle() / 360 * 2 * itk::Math::pi);
  float part_multiplicator = tan_phi * config->GetTimeSpacing() *
    config->GetSpeedOfSound() / config->GetPitchInMeters() * inputL / (float)config->GetTransducerElements();
  float apod_mult = 1;

  float mult = 0;

  short usedLines = (maxLine - minLine);

  float percentOfImageReconstructed = (float)(config->GetReconstructionDepth()) /
    (float)(inputS * config->GetSpeedOfSound() * config->GetTimeSpacing() / (float)(2 - (int)config->GetIsPhotoacousticImage()));
  percentOfImageReconstructed = percentOfImageReconstructed <= 1 ? percentOfImageReconstructed : 1;

  l_i = (float)line / outputL * inputL;

  for (short sample = 0; sample < outputS; ++sample)
  {
    s_i = (float)sample / outputS * inputS / (float)(2 - (int)config->GetIsPhotoacousticImage()) * percentOfImageReconstructed;

    part = part_multiplicator*s_i;

    if (part < 1)
      part = 1;

    maxLine = (short)std::min((l_i + part) + 1, inputL);
    minLine = (short)std::max((l_i - part), 0.0f);
    usedLines = (maxLine - minLine);

    apod_mult = (float)apodArraySize / (float)usedLines;

    //calculate the AddSamples beforehand to save some time
    short* AddSample = new short[maxLine - minLine];
    for (short l_s = 0; l_s < maxLine - minLine; ++l_s)
    {
      AddSample[l_s] = (short)sqrt(
        pow(s_i, 2)
        +
        pow((1 / (config->GetTimeSpacing()*config->GetSpeedOfSound()) *
        (((float)minLine + (float)l_s - l_i)*config->GetPitchInMeters()*(float)config->GetTransducerElements()) / inputL), 2)
      ) + (1 - config->GetIsPhotoacousticImage())*s_i;
    }

    float s_1 = 0;
    float s_2 = 0;

    for (short l_s1 = minLine; l_s1 < maxLine - 1; ++l_s1)
    {
      if (AddSample[l_s1 - minLine] < inputS && AddSample[l_s1 - minLine] >= 0)
      {
        for (short l_s2 = l_s1 + 1; l_s2 < maxLine; ++l_s2)
        {
          if (AddSample[l_s2 - minLine] < inputS && AddSample[l_s2 - minLine] >= 0)
          {
            s_2 = input[l_s2 + AddSample[l_s2 - minLine] * (short)inputL];
            s_1 = input[l_s1 + AddSample[l_s1 - minLine] * (short)inputL];

            mult = s_2 * apodisation[(int)((l_s2 - minLine)*apod_mult)] * s_1 * apodisation[(int)((l_s1 - minLine)*apod_mult)];
            output[sample*(short)outputL + line] += sqrt(fabs(mult)) * ((mult > 0) - (mult < 0));
          }
        }
      }
      else
        --usedLines;
    }

    output[sample*(short)outputL + line] = output[sample*(short)outputL + line] / (float)(pow(usedLines, 2) - (usedLines - 1));

    delete[] AddSample;
  }
}

void mitk::BeamformingUtils::sDMASQuadraticLine(
  float* input, float* output, float inputDim[2], float outputDim[2],
  const short& line, const mitk::BeamformingSettings::Pointer config)
{
  const float* apodisation = config->GetApodizationFunction();
  const short apodArraySize = config->GetApodizationArraySize();

  float& inputS = inputDim[1];
  float& inputL = inputDim[0];

  float& outputS = outputDim[1];
  float& outputL = outputDim[0];

  short maxLine = 0;
  short minLine = 0;
  float delayMultiplicator = 0;
  float l_i = 0;
  float s_i = 0;

  float part = 0.07 * inputL;
  float tan_phi = std::tan(config->GetAngle() / 360 * 2 * itk::Math::pi);
  float part_multiplicator = tan_phi * config->GetTimeSpacing() * config->GetSpeedOfSound() /
    config->GetPitchInMeters() * inputL / (float)config->GetTransducerElements();
  float apod_mult = 1;

  float mult = 0;
  short usedLines = (maxLine - minLine);

  float percentOfImageReconstructed = (float)(config->GetReconstructionDepth()) /
    (float)(inputS * config->GetSpeedOfSound() * config->GetTimeSpacing() / (float)(2 - (int)config->GetIsPhotoacousticImage()));
  percentOfImageReconstructed = percentOfImageReconstructed <= 1 ? percentOfImageReconstructed : 1;

  l_i = (float)line / outputL * inputL;

  for (short sample = 0; sample < outputS; ++sample)
  {
    s_i = (float)sample / outputS * inputS / (float)(2 - (int)config->GetIsPhotoacousticImage()) * percentOfImageReconstructed;

    part = part_multiplicator*s_i;

    if (part < 1)
      part = 1;

    maxLine = (short)std::min((l_i + part) + 1, inputL);
    minLine = (short)std::max((l_i - part), 0.0f);
    usedLines = (maxLine - minLine);

    apod_mult = (float)apodArraySize / (float)usedLines;

    delayMultiplicator = pow((1 / (config->GetTimeSpacing()*config->GetSpeedOfSound()) *
      (config->GetPitchInMeters()*config->GetTransducerElements()) / inputL), 2) / s_i / 2;

    //calculate the AddSamples beforehand to save some time
    short* AddSample = new short[maxLine - minLine];
    for (short l_s = 0; l_s < maxLine - minLine; ++l_s)
    {
      AddSample[l_s] = (short)(delayMultiplicator * pow((minLine + l_s - l_i), 2) + s_i) +
        (1 - config->GetIsPhotoacousticImage())*s_i;
    }

    float s_1 = 0;
    float s_2 = 0;
    float sign = 0;

    for (short l_s1 = minLine; l_s1 < maxLine - 1; ++l_s1)
    {
      if (AddSample[l_s1 - minLine] < inputS && AddSample[l_s1 - minLine] >= 0)
      {
        s_1 = input[l_s1 + AddSample[l_s1 - minLine] * (short)inputL];
        sign += s_1;

        for (short l_s2 = l_s1 + 1; l_s2 < maxLine; ++l_s2)
        {
          if (AddSample[l_s2 - minLine] < inputS && AddSample[l_s2 - minLine] >= 0)
          {
            s_2 = input[l_s2 + AddSample[l_s2 - minLine] * (short)inputL];

            mult = s_2 * apodisation[(int)((l_s2 - minLine)*apod_mult)] * s_1 * apodisation[(int)((l_s1 - minLine)*apod_mult)];
            output[sample*(short)outputL + line] += sqrt(fabs(mult)) * ((mult > 0) - (mult < 0));
          }
        }
      }
      else
        --usedLines;
    }

    output[sample*(short)outputL + line] = output[sample*(short)outputL + line] / (float)(pow(usedLines, 2) - (usedLines - 1)) * ((sign > 0) - (sign < 0));

    delete[] AddSample;
  }
}

void mitk::BeamformingUtils::sDMASSphericalLine(
  float* input, float* output, float inputDim[2], float outputDim[2],
  const short& line, const mitk::BeamformingSettings::Pointer config)
{
  const float* apodisation = config->GetApodizationFunction();
  const short apodArraySize = config->GetApodizationArraySize();

  float& inputS = inputDim[1];
  float& inputL = inputDim[0];

  float& outputS = outputDim[1];
  float& outputL = outputDim[0];

  short maxLine = 0;
  short minLine = 0;
  float l_i = 0;
  float s_i = 0;

  float part = 0.07 * inputL;
  float tan_phi = std::tan(config->GetAngle() / 360 * 2 * itk::Math::pi);
  float part_multiplicator = tan_phi * config->GetTimeSpacing() * config->GetSpeedOfSound() /
    config->GetPitchInMeters() * inputL / (float)config->GetTransducerElements();
  float apod_mult = 1;

  float mult = 0;

  short usedLines = (maxLine - minLine);

  float percentOfImageReconstructed = (float)(config->GetReconstructionDepth()) /
    (float)(inputS * config->GetSpeedOfSound() * config->GetTimeSpacing() / (float)(2 - (int)config->GetIsPhotoacousticImage()));
  percentOfImageReconstructed = percentOfImageReconstructed <= 1 ? percentOfImageReconstructed : 1;

  l_i = (float)line / outputL * inputL;

  for (short sample = 0; sample < outputS; ++sample)
  {
    s_i = (float)sample / outputS * inputS / (float)(2 - (int)config->GetIsPhotoacousticImage()) * percentOfImageReconstructed;

    part = part_multiplicator*s_i;

    if (part < 1)
      part = 1;

    maxLine = (short)std::min((l_i + part) + 1, inputL);
    minLine = (short)std::max((l_i - part), 0.0f);
    usedLines = (maxLine - minLine);

    apod_mult = (float)apodArraySize / (float)usedLines;

    //calculate the AddSamples beforehand to save some time
    short* AddSample = new short[maxLine - minLine];
    for (short l_s = 0; l_s < maxLine - minLine; ++l_s)
    {
      AddSample[l_s] = (short)sqrt(
        pow(s_i, 2)
        +
        pow((1 / (config->GetTimeSpacing()*config->GetSpeedOfSound()) *
        (((float)minLine + (float)l_s - l_i)*config->GetPitchInMeters()*(float)config->GetTransducerElements()) / inputL), 2)
      ) + (1 - config->GetIsPhotoacousticImage())*s_i;
    }

    float s_1 = 0;
    float s_2 = 0;
    float sign = 0;

    for (short l_s1 = minLine; l_s1 < maxLine - 1; ++l_s1)
    {
      if (AddSample[l_s1 - minLine] < inputS && AddSample[l_s1 - minLine] >= 0)
      {
        s_1 = input[l_s1 + AddSample[l_s1 - minLine] * (short)inputL];
        sign += s_1;

        for (short l_s2 = l_s1 + 1; l_s2 < maxLine; ++l_s2)
        {
          if (AddSample[l_s2 - minLine] < inputS && AddSample[l_s2 - minLine] >= 0)
          {
            s_2 = input[l_s2 + AddSample[l_s2 - minLine] * (short)inputL];

            mult = s_2 * apodisation[(int)((l_s2 - minLine)*apod_mult)] * s_1 * apodisation[(int)((l_s1 - minLine)*apod_mult)];
            output[sample*(short)outputL + line] += sqrt(fabs(mult)) * ((mult > 0) - (mult < 0));
          }
        }
      }
      else
        --usedLines;
    }

    output[sample*(short)outputL + line] = output[sample*(short)outputL + line] / (float)(pow(usedLines, 2) - (usedLines - 1)) * ((sign > 0) - (sign < 0));

    delete[] AddSample;
  }
}
