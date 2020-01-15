/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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

unsigned short* mitk::BeamformingUtils::MinMaxLines(const mitk::BeamformingSettings::Pointer config)
{
  unsigned int outputL = (unsigned int)config->GetReconstructionLines();
  unsigned int outputS = (unsigned int)config->GetSamplesPerLine();

  unsigned short* dDest = new unsigned short[outputL * outputS * 2];

  unsigned int inputL = (unsigned int)config->GetInputDim()[0];

  float horizontalExtent = config->GetHorizontalExtent();
  float verticalExtent = config->GetReconstructionDepth();

  float partMult = (tan(config->GetAngle() / 360 * 2 * itk::Math::pi) *
    ((config->GetSpeedOfSound() * config->GetTimeSpacing())) /
    (config->GetPitchInMeters() * config->GetTransducerElements())) * inputL;
  float totalSamples_i = (float)(config->GetReconstructionDepth()) / (float)(config->GetSpeedOfSound() * config->GetTimeSpacing());

  totalSamples_i = totalSamples_i <= config->GetInputDim()[1] ? totalSamples_i : config->GetInputDim()[1];

  if ((int)config->GetGeometry() == 0) // if this is raw data from a linear probe geometry
  {
    for (unsigned int x = 0; x < outputL; ++x)
    {
      for (unsigned int y = 0; y < outputS; ++y)
      {
        float l_i = (float)x / outputL * inputL;
        float s_i = (float)y / (float)outputS * totalSamples_i;

        float part = partMult * s_i;
        if (part < 1)
          part = 1;
        unsigned short maxLine = std::min((l_i + part) + 1, (float)inputL);
        unsigned short minLine = std::max((l_i - part), 0.0f);

        dDest[y * 2 * outputL + 2 * x] = (unsigned short)minLine; //minLine
        dDest[y * 2 * outputL + 2 * x + 1] = (unsigned short)maxLine; //maxLine
      }
    }

  }
  else // if this is *not* raw data from a linear probe geometry (currently meaning its a concave geometry)
  {
    float probeRadius = config->GetProbeRadius();
    float* elementHeights = config->GetElementHeights();
    float* elementPositions = config->GetElementPositions();

    float sin_deg = std::sin(config->GetAngle() / 360 * 2 * itk::Math::pi);

    float x_center_pos = horizontalExtent / 2.0;
    float y_center_pos = probeRadius;

    for (unsigned int x = 0; x < outputL; ++x)
    {
      for (unsigned int y = 0; y < outputS; ++y)
      {
        float x_cm = (float)x / outputL * horizontalExtent; // x*Xspacing
        float y_cm = (float)y / (float)outputS * verticalExtent; // y*Yspacing

        unsigned int maxLine = inputL;
        unsigned int minLine = 0;

        for (unsigned int l_s = minLine; l_s <= inputL; l_s += 1)
        {
          float x_sensor_pos = elementPositions[l_s];
          float y_sensor_pos = elementHeights[l_s];

          float distance_sensor_target = sqrt((x_cm - x_sensor_pos)*(x_cm - x_sensor_pos)
                                              + (y_cm - y_sensor_pos)*(y_cm - y_sensor_pos));

          // solving line equation
          float center_to_sensor_a = y_sensor_pos - y_center_pos;
          float center_to_sensor_b = x_center_pos - x_sensor_pos;
          float center_to_sensor_c = -(center_to_sensor_a * x_center_pos + center_to_sensor_b * y_center_pos);
          float distance_to_sensor_direction = std::fabs((center_to_sensor_a * x_cm
                                                          + center_to_sensor_b * y_cm
                                                          + center_to_sensor_c)) /
                       (sqrt(center_to_sensor_a*center_to_sensor_a + center_to_sensor_b*center_to_sensor_b));

          if (distance_to_sensor_direction < sin_deg*distance_sensor_target)
          {
            minLine = l_s;
            break;
          }
        }

        for (unsigned int l_s = maxLine - 1; l_s > minLine; l_s -= 1) // start with maxline-1 otherwise elementPositions[] will go out of range
        {
          float x_sensor_pos = elementPositions[l_s];
          float y_sensor_pos = elementHeights[l_s];

          float distance_sensor_target = sqrt((x_cm - x_sensor_pos)*(x_cm - x_sensor_pos)
                                              + (y_cm - y_sensor_pos)*(y_cm - y_sensor_pos));

          // solving line equation
          float center_to_sensor_a = y_sensor_pos - y_center_pos;
          float center_to_sensor_b = x_center_pos - x_sensor_pos;
          float center_to_sensor_c = -(center_to_sensor_a * x_center_pos + center_to_sensor_b * y_center_pos);
          float distance_to_sensor_direction = std::fabs((center_to_sensor_a * x_cm
                                                          + center_to_sensor_b * y_cm
                                                          + center_to_sensor_c)) /
                       (sqrt(center_to_sensor_a*center_to_sensor_a + center_to_sensor_b*center_to_sensor_b));

          if (distance_to_sensor_direction < sin_deg*distance_sensor_target)
          {
            maxLine = l_s;
            break;
          }
        }
        dDest[y * 2 * outputL + 2 * x] = (unsigned short)minLine; //minLine
        dDest[y * 2 * outputL + 2 * x + 1] = (unsigned short)maxLine; //maxLine
      }
    }
  }
  return dDest;
}

void mitk::BeamformingUtils::DASSphericalLine(
  float* input, float* output, float inputDim[2], float outputDim[2],
  const short& line, const mitk::BeamformingSettings::Pointer config)
{
  const float* apodisation = config->GetApodizationFunction();
  const short apodArraySize = config->GetApodizationArraySize();

  const float* elementHeights = config->GetElementHeights();
  const float* elementPositions = config->GetElementPositions();

  float& inputS = inputDim[1];
  float& inputL = inputDim[0];

  float& outputS = outputDim[1];
  float& outputL = outputDim[0];

  short AddSample = 0;
  short maxLine = 0;
  short minLine = 0;
  float l_p = 0;
  float s_i = 0;

  float apod_mult = 1;

  short usedLines = (maxLine - minLine);

  float totalSamples_i = (float)(config->GetReconstructionDepth()) / (float)(config->GetSpeedOfSound() * config->GetTimeSpacing());
  totalSamples_i = totalSamples_i <= inputS ? totalSamples_i : inputS;

  l_p = (float)line / outputL * config->GetHorizontalExtent();

  for (short sample = 0; sample < outputS; ++sample)
  {
    s_i = (float)sample / outputS * totalSamples_i;

    minLine = config->GetMinMaxLines()[2*sample*(short)outputL + 2*line];
    maxLine = config->GetMinMaxLines()[2*sample*(short)outputL + 2*line + 1];
    usedLines = (maxLine - minLine);

    apod_mult = (float)apodArraySize / (float)usedLines;

    for (short l_s = minLine; l_s < maxLine; ++l_s)
    {
      AddSample = (int)sqrt(
        pow(s_i-elementHeights[l_s]/(config->GetSpeedOfSound()*config->GetTimeSpacing()), 2)
        +
        pow((1 / (config->GetTimeSpacing()*config->GetSpeedOfSound())) * (l_p - elementPositions[l_s]), 2)
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

void mitk::BeamformingUtils::DMASSphericalLine(
  float* input, float* output, float inputDim[2], float outputDim[2],
  const short& line, const mitk::BeamformingSettings::Pointer config)
{
  const float* apodisation = config->GetApodizationFunction();
  const short apodArraySize = config->GetApodizationArraySize();

  const float* elementHeights = config->GetElementHeights();
  const float* elementPositions = config->GetElementPositions();

  float& inputS = inputDim[1];
  float& inputL = inputDim[0];

  float& outputS = outputDim[1];
  float& outputL = outputDim[0];

  short maxLine = 0;
  short minLine = 0;
  float l_p = 0;
  float s_i = 0;

  float apod_mult = 1;

  float mult = 0;

  short usedLines = (maxLine - minLine);

  float totalSamples_i = (float)(config->GetReconstructionDepth()) /
    (float)(config->GetSpeedOfSound() * config->GetTimeSpacing());
  totalSamples_i = totalSamples_i <= inputS ? totalSamples_i : inputS;

  l_p = (float)line / outputL * config->GetHorizontalExtent();

  for (short sample = 0; sample < outputS; ++sample)
  {
    s_i = (float)sample / outputS * totalSamples_i;

    minLine = config->GetMinMaxLines()[2 * sample*(short)outputL + 2 * line];
    maxLine = config->GetMinMaxLines()[2 * sample*(short)outputL + 2 * line + 1];
    usedLines = (maxLine - minLine);

    apod_mult = (float)apodArraySize / (float)usedLines;

    //calculate the AddSamples beforehand to save some time
    short* AddSample = new short[maxLine - minLine];
    for (short l_s = 0; l_s < maxLine - minLine; ++l_s)
    {
      AddSample[l_s] = (int)sqrt(
        pow(s_i - elementHeights[l_s + minLine] / (config->GetSpeedOfSound()*config->GetTimeSpacing()), 2)
        +
        pow((1 / (config->GetTimeSpacing()*config->GetSpeedOfSound())) * (l_p - elementPositions[l_s + minLine]), 2)
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

void mitk::BeamformingUtils::sDMASSphericalLine(
  float* input, float* output, float inputDim[2], float outputDim[2],
  const short& line, const mitk::BeamformingSettings::Pointer config)
{
  const float* apodisation = config->GetApodizationFunction();
  const short apodArraySize = config->GetApodizationArraySize();

  const float* elementHeights = config->GetElementHeights();
  const float* elementPositions = config->GetElementPositions();

  float& inputS = inputDim[1];
  float& inputL = inputDim[0];

  float& outputS = outputDim[1];
  float& outputL = outputDim[0];

  short maxLine = 0;
  short minLine = 0;
  float l_p = 0;
  float s_i = 0;

  float apod_mult = 1;

  float mult = 0;

  short usedLines = (maxLine - minLine);

  float totalSamples_i = (float)(config->GetReconstructionDepth()) /
    (float)(config->GetSpeedOfSound() * config->GetTimeSpacing());
  totalSamples_i = totalSamples_i <= inputS ? totalSamples_i : inputS;

  l_p = (float)line / outputL * config->GetHorizontalExtent();

  for (short sample = 0; sample < outputS; ++sample)
  {
    s_i = (float)sample / outputS * totalSamples_i;

    minLine = config->GetMinMaxLines()[2 * sample*(short)outputL + 2 * line];
    maxLine = config->GetMinMaxLines()[2 * sample*(short)outputL + 2 * line + 1];
    usedLines = (maxLine - minLine);

    apod_mult = (float)apodArraySize / (float)usedLines;

    //calculate the AddSamples beforehand to save some time
    short* AddSample = new short[maxLine - minLine];
    for (short l_s = 0; l_s < maxLine - minLine; ++l_s)
    {
      AddSample[l_s] = (int)sqrt(
        pow(s_i - elementHeights[l_s + minLine] / (config->GetSpeedOfSound()*config->GetTimeSpacing()), 2)
        +
        pow((1 / (config->GetTimeSpacing()*config->GetSpeedOfSound())) * (l_p - elementPositions[l_s + minLine]), 2)
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
