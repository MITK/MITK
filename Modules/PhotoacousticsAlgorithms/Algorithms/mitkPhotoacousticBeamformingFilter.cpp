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

#define _USE_MATH_DEFINES

#include "mitkPhotoacousticBeamformingFilter.h"
#include "mitkProperties.h"
#include "mitkImageReadAccessor.h"
#include <algorithm>
#include <itkImageIOBase.h>
#include <chrono>
#include <cmath>
#include <thread>
#include <itkImageIOBase.h>
#include "mitkImageCast.h"
#include <mitkPhotoacousticOCLBeamformer.h>

mitk::BeamformingFilter::BeamformingFilter() : m_OutputData(nullptr), m_InputData(nullptr)
{
  this->SetNumberOfIndexedInputs(1);
  this->SetNumberOfRequiredInputs(1);

  m_ProgressHandle = [](int, std::string) {};
}

void mitk::BeamformingFilter::SetProgressHandle(std::function<void(int, std::string)> progressHandle)
{
  m_ProgressHandle = progressHandle;
}

mitk::BeamformingFilter::~BeamformingFilter()
{
}

void mitk::BeamformingFilter::GenerateInputRequestedRegion()
{
  Superclass::GenerateInputRequestedRegion();

  mitk::Image* output = this->GetOutput();
  mitk::Image* input = const_cast<mitk::Image *> (this->GetInput());
  if (!output->IsInitialized())
  {
    return;
  }

  input->SetRequestedRegionToLargestPossibleRegion();

  //GenerateTimeInInputRegion(output, input);
}

void mitk::BeamformingFilter::GenerateOutputInformation()
{
  mitk::Image::ConstPointer input = this->GetInput();
  mitk::Image::Pointer output = this->GetOutput();

  if ((output->IsInitialized()) && (this->GetMTime() <= m_TimeOfHeaderInitialization.GetMTime()))
    return;

  itkDebugMacro(<< "GenerateOutputInformation()");

  unsigned int dim[] = { m_Conf.ReconstructionLines, m_Conf.SamplesPerLine, input->GetDimension(2) };
  output->Initialize(mitk::MakeScalarPixelType<float>(), 3, dim);

  mitk::Vector3D spacing;
  spacing[0] = m_Conf.Pitch * m_Conf.TransducerElements * 1000 / m_Conf.ReconstructionLines;
  spacing[1] = m_Conf.RecordTime / 2 * m_Conf.SpeedOfSound * 1000 / m_Conf.SamplesPerLine;
  spacing[2] = 1;

  output->GetGeometry()->SetSpacing(spacing);
  output->GetGeometry()->Modified();
  output->SetPropertyList(input->GetPropertyList()->Clone());

  m_TimeOfHeaderInitialization.Modified();
}

void mitk::BeamformingFilter::GenerateData()
{
  GenerateOutputInformation();
  mitk::Image::Pointer input = this->GetInput();
  mitk::Image::Pointer output = this->GetOutput();

  if (!output->IsInitialized())
    return;

  float inputDim[2] = { (float)input->GetDimension(0), (float)input->GetDimension(1) };
  float outputDim[2] = { (float)output->GetDimension(0), (float)output->GetDimension(1) };

  unsigned short chunkSize = 2; // TODO: make this slightly less arbitrary

  unsigned int oclOutputDim[3] = { output->GetDimension(0), output->GetDimension(1), output->GetDimension(2) };

  unsigned int oclOutputDimChunk[3] = { output->GetDimension(0), output->GetDimension(1), chunkSize};
  unsigned int oclInputDimChunk[3] = { input->GetDimension(0), input->GetDimension(1), chunkSize};

  unsigned int oclOutputDimLastChunk[3] = { output->GetDimension(0), output->GetDimension(1), input->GetDimension(2) % chunkSize };
  unsigned int oclInputDimLastChunk[3] = { input->GetDimension(0), input->GetDimension(1), input->GetDimension(2) % chunkSize };

  const int apodArraySize = m_Conf.TransducerElements * 4; // set the resolution of the apodization array
  float* ApodWindow;

  // calculate the appropiate apodization window
  switch (m_Conf.Apod)
  {
  case beamformingSettings::Apodization::Hann:
    ApodWindow = VonHannFunction(apodArraySize);
    break;
  case beamformingSettings::Apodization::Hamm:
    ApodWindow = HammFunction(apodArraySize);
    break;
  case beamformingSettings::Apodization::Box:
    ApodWindow = BoxFunction(apodArraySize);
    break;
  default:
    ApodWindow = BoxFunction(apodArraySize);
    break;
  }

  int progInterval = output->GetDimension(2) / 20 > 1 ? output->GetDimension(2) / 20 : 1;
  // the interval at which we update the gui progress bar

  auto begin = std::chrono::high_resolution_clock::now(); // debbuging the performance...
  if (!m_Conf.UseGPU)
  {
    for (unsigned int i = 0; i < output->GetDimension(2); ++i) // seperate Slices should get Beamforming seperately applied
    {
      mitk::ImageReadAccessor inputReadAccessor(input, input->GetSliceData(i));

      // first, we check whether the dara is float, other formats are unsupported
      if (input->GetPixelType().GetTypeAsString() == "scalar (float)" || input->GetPixelType().GetTypeAsString() == " (float)")
      {
        m_InputData = (float*)inputReadAccessor.GetData();
      }
      else
      {
        MITK_INFO << "Pixel type is not float, abort";
        return;
      }

      m_OutputData = new float[m_Conf.ReconstructionLines*m_Conf.SamplesPerLine];

      // fill the image with zeros
      for (int l = 0; l < outputDim[0]; ++l)
      {
        for (int s = 0; s < outputDim[1]; ++s)
        {
          m_OutputData[l*(short)outputDim[1] + s] = 0;
        }
      }

      std::thread *threads = new std::thread[(short)outputDim[0]];

      // every line will be beamformed in a seperate thread
      if (m_Conf.Algorithm == beamformingSettings::BeamformingAlgorithm::DAS)
      {
        if (m_Conf.DelayCalculationMethod == beamformingSettings::DelayCalc::QuadApprox)
        {
          for (short line = 0; line < outputDim[0]; ++line)
          {
            threads[line] = std::thread(&BeamformingFilter::DASQuadraticLine, this, m_InputData, m_OutputData, inputDim, outputDim, line, ApodWindow, apodArraySize);
          }
        }
        else if (m_Conf.DelayCalculationMethod == beamformingSettings::DelayCalc::Spherical)
        {
          for (short line = 0; line < outputDim[0]; ++line)
          {
            threads[line] = std::thread(&BeamformingFilter::DASSphericalLine, this, m_InputData, m_OutputData, inputDim, outputDim, line, ApodWindow, apodArraySize);
          }
        }
      }
      else if (m_Conf.Algorithm == beamformingSettings::BeamformingAlgorithm::DMAS)
      {
        if (m_Conf.DelayCalculationMethod == beamformingSettings::DelayCalc::QuadApprox)
        {
          for (short line = 0; line < outputDim[0]; ++line)
          {
            threads[line] = std::thread(&BeamformingFilter::DMASQuadraticLine, this, m_InputData, m_OutputData, inputDim, outputDim, line, ApodWindow, apodArraySize);
          }
        }
        else if (m_Conf.DelayCalculationMethod == beamformingSettings::DelayCalc::Spherical)
        {
          for (short line = 0; line < outputDim[0]; ++line)
          {
            threads[line] = std::thread(&BeamformingFilter::DMASSphericalLine, this, m_InputData, m_OutputData, inputDim, outputDim, line, ApodWindow, apodArraySize);
          }
        }
      }
      // wait for all lines to finish
      for (short line = 0; line < outputDim[0]; ++line)
      {
        threads[line].join();
      }

      output->SetSlice(m_OutputData, i);

      if (i % progInterval == 0)
        m_ProgressHandle((int)((i + 1) / (float)output->GetDimension(2) * 100), "performing reconstruction");

      delete[] m_OutputData;
      m_OutputData = nullptr;
      m_InputData = nullptr;
    }
  }
  #ifdef PHOTOACOUSTICS_USE_GPU
  else
  {
    mitk::PhotoacousticOCLBeamformer::Pointer m_oclFilter = mitk::PhotoacousticOCLBeamformer::New();

    try
    {
      if (m_Conf.Algorithm == beamformingSettings::BeamformingAlgorithm::DAS)
      {
        if (m_Conf.DelayCalculationMethod == beamformingSettings::DelayCalc::QuadApprox)
          m_oclFilter->SetAlgorithm(PhotoacousticOCLBeamformer::BeamformingAlgorithm::DASQuad, true);
        else if (m_Conf.DelayCalculationMethod == beamformingSettings::DelayCalc::Spherical)
          m_oclFilter->SetAlgorithm(PhotoacousticOCLBeamformer::BeamformingAlgorithm::DASSphe, true);
      }
      else if (m_Conf.Algorithm == beamformingSettings::BeamformingAlgorithm::DMAS)
      {
        if (m_Conf.DelayCalculationMethod == beamformingSettings::DelayCalc::QuadApprox)
          m_oclFilter->SetAlgorithm(PhotoacousticOCLBeamformer::BeamformingAlgorithm::DMASQuad, true);
        else if (m_Conf.DelayCalculationMethod == beamformingSettings::DelayCalc::Spherical)
          m_oclFilter->SetAlgorithm(PhotoacousticOCLBeamformer::BeamformingAlgorithm::DMASSphe, true);
      }
      m_oclFilter->SetApodisation(ApodWindow, apodArraySize);
      m_oclFilter->SetOutputDim(oclOutputDimChunk);
      m_oclFilter->SetBeamformingParameters(m_Conf.SpeedOfSound, m_Conf.TimeSpacing, m_Conf.Pitch, m_Conf.Angle, m_Conf.Photoacoustic, m_Conf.TransducerElements);

      mitk::ImageReadAccessor inputReadAccessor(input);

      // first, we check whether the data is float, other formats are unsupported
      if (input->GetPixelType().GetTypeAsString() == "scalar (float)" || input->GetPixelType().GetTypeAsString() == " (float)")
      {
        m_InputData = (float*)inputReadAccessor.GetData();
      }
      else
      {
        MITK_INFO << "Pixel type is not float, abort";
        return;
      }

      if (chunkSize < oclOutputDim[2])
      {
        unsigned short curChunkSize = chunkSize;

        for (unsigned int i = 0; i < ceil((float)oclOutputDim[2] / (float)chunkSize); ++i)
        {
          m_ProgressHandle(100 * ((float)(i * chunkSize) / (float)oclOutputDim[2]), "performing reconstruction");

          if ((oclOutputDim[2]) - (i * chunkSize) >= chunkSize)
          {
            m_oclFilter->SetInput((void*)&m_InputData[i * chunkSize * input->GetDimension(0) * input->GetDimension(1)], oclInputDimChunk, sizeof(float));
          }
          else
          {
            m_oclFilter->SetInput((void*)&m_InputData[i * chunkSize * input->GetDimension(0) * input->GetDimension(1)], oclInputDimLastChunk, sizeof(float));
            curChunkSize = (unsigned short)oclOutputDimLastChunk[2]; // the last chunk might have a different size!
          }
          
          m_oclFilter->Update();
          float* out = (float*)m_oclFilter->GetOutput();

          for (unsigned short s = 0; s < curChunkSize; ++s)
          {
            output->SetImportSlice( (void*)&(out[s * oclOutputDim[0] * oclOutputDim[1]]), i * chunkSize + s, 0, 0, mitk::Image::ReferenceMemory);
          }
        }
      }
      else
      {
        m_ProgressHandle(50, "performing reconstruction");

        m_oclFilter->SetOutputDim(oclOutputDim);
        m_oclFilter->SetInput(input);
        m_oclFilter->Update();

        float* out = (float*)m_oclFilter->GetOutput();
        output->SetImportVolume((void*)out, 0, 0, mitk::Image::ReferenceMemory);
      }
    }
    catch (mitk::Exception &e)
    {
      std::string errorMessage = "Caught unexpected exception ";
      errorMessage.append(e.what());
      MITK_ERROR << errorMessage;
    }
  }
  #endif
  m_TimeOfHeaderInitialization.Modified();

  auto end = std::chrono::high_resolution_clock::now();
  MITK_INFO << "Beamforming of " << output->GetDimension(2) << " Images completed in " << ((float)std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count()) / 1000000 << "ms" << std::endl;
}

void mitk::BeamformingFilter::Configure(beamformingSettings settings)
{
  m_Conf = settings;
}

float* mitk::BeamformingFilter::VonHannFunction(int samples)
{
  float* ApodWindow = new float[samples];

  for (int n = 0; n < samples; ++n)
  {
    ApodWindow[n] = (1 - cos(2 * M_PI * n / (samples - 1))) / 2;
  }

  return ApodWindow;
}

float* mitk::BeamformingFilter::HammFunction(int samples)
{
  float* ApodWindow = new float[samples];

  for (int n = 0; n < samples; ++n)
  {
    ApodWindow[n] = 0.54 - 0.46*cos(2 * M_PI*n / (samples - 1));
  }

  return ApodWindow;
}

float* mitk::BeamformingFilter::BoxFunction(int samples)
{
  float* ApodWindow = new float[samples];

  for (int n = 0; n < samples; ++n)
  {
    ApodWindow[n] = 1;
  }

  return ApodWindow;
}

void mitk::BeamformingFilter::DASQuadraticLine(float* input, float* output, float inputDim[2], float outputDim[2], const short& line, float* apodisation, const short& apodArraySize)
{
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

  float part = 0.07 * inputL;
  float tan_phi = std::tan(m_Conf.Angle / 360 * 2 * M_PI);
  float part_multiplicator = tan_phi * m_Conf.TimeSpacing * m_Conf.SpeedOfSound / m_Conf.Pitch * m_Conf.ReconstructionLines / m_Conf.TransducerElements;
  float apod_mult = 1;

  short usedLines = (maxLine - minLine);

  //quadratic delay
  l_i = line / outputL * inputL;

  for (short sample = 0; sample < outputS; ++sample)
  {
    s_i = (float)sample / outputS * inputS / 2;

    part = part_multiplicator*s_i;

    if (part < 1)
      part = 1;

    maxLine = (short)std::min((l_i + part) + 1, inputL);
    minLine = (short)std::max((l_i - part), 0.0f);
    usedLines = (maxLine - minLine);

    apod_mult = apodArraySize / (maxLine - minLine);

    delayMultiplicator = pow((1 / (m_Conf.TimeSpacing*m_Conf.SpeedOfSound) * (m_Conf.Pitch*m_Conf.TransducerElements) / inputL), 2) / s_i / 2;

    for (short l_s = minLine; l_s < maxLine; ++l_s)
    {
      AddSample = delayMultiplicator * pow((l_s - l_i), 2) + s_i + (1 - m_Conf.Photoacoustic)*s_i;
      if (AddSample < inputS && AddSample >= 0) 
        output[sample*(short)outputL + line] += input[l_s + AddSample*(short)inputL] * apodisation[(short)((l_s - minLine)*apod_mult)];
      else
        --usedLines;
    }
    output[sample*(short)outputL + line] = output[sample*(short)outputL + line] / usedLines;
  }
}

void mitk::BeamformingFilter::DASSphericalLine(float* input, float* output, float inputDim[2], float outputDim[2], const short& line, float* apodisation, const short& apodArraySize)
{
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
  float tan_phi = std::tan(m_Conf.Angle / 360 * 2 * M_PI);
  float part_multiplicator = tan_phi * m_Conf.TimeSpacing * m_Conf.SpeedOfSound / m_Conf.Pitch * m_Conf.ReconstructionLines / m_Conf.TransducerElements;
  float apod_mult = 1;

  short usedLines = (maxLine - minLine);

  //exact delay

  l_i = (float)line / outputL * inputL;

  for (short sample = 0; sample < outputS; ++sample)
  {
    s_i = (float)sample / outputS * inputS / 2;

    part = part_multiplicator*s_i;

    if (part < 1)
      part = 1;

    maxLine = (short)std::min((l_i + part) + 1, inputL);
    minLine = (short)std::max((l_i - part), 0.0f);
    usedLines = (maxLine - minLine);

    apod_mult = apodArraySize / (maxLine - minLine);

    for (short l_s = minLine; l_s < maxLine; ++l_s)
    {
      AddSample = (int)sqrt(
        pow(s_i, 2)
        +
        pow((1 / (m_Conf.TimeSpacing*m_Conf.SpeedOfSound) * ((l_s - l_i)*m_Conf.Pitch*m_Conf.TransducerElements) / inputL), 2)
      ) + (1 - m_Conf.Photoacoustic)*s_i;
      if (AddSample < inputS && AddSample >= 0) 
        output[sample*(short)outputL + line] += input[l_s + AddSample*(short)inputL] * apodisation[(short)((l_s - minLine)*apod_mult)];
      else
        --usedLines;
    }
    output[sample*(short)outputL + line] = output[sample*(short)outputL + line] / usedLines;
  }
}

void mitk::BeamformingFilter::DMASQuadraticLine(float* input, float* output, float inputDim[2], float outputDim[2], const short& line, float* apodisation, const short& apodArraySize)
{
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
  float tan_phi = std::tan(m_Conf.Angle / 360 * 2 * M_PI);
  float part_multiplicator = tan_phi * m_Conf.TimeSpacing * m_Conf.SpeedOfSound / m_Conf.Pitch * m_Conf.ReconstructionLines / m_Conf.TransducerElements;
  float apod_mult = 1;

  float mult = 0;
  short usedLines = (maxLine - minLine);

  //quadratic delay
  l_i = line / outputL * inputL;

  for (short sample = 0; sample < outputS; ++sample)
  {
    s_i = sample / outputS * inputS / 2;

    part = part_multiplicator*s_i;

    if (part < 1)
      part = 1;

    maxLine = (short)std::min((l_i + part) + 1, inputL);
    minLine = (short)std::max((l_i - part), 0.0f);
    usedLines = (maxLine - minLine);

    apod_mult = apodArraySize / (maxLine - minLine);

    delayMultiplicator = pow((1 / (m_Conf.TimeSpacing*m_Conf.SpeedOfSound) * (m_Conf.Pitch*m_Conf.TransducerElements) / inputL), 2) / s_i / 2;

    //calculate the AddSamples beforehand to save some time
    short* AddSample = new short[maxLine - minLine];
    for (short l_s = 0; l_s < maxLine - minLine; ++l_s)
    {
      AddSample[l_s] = (short)(delayMultiplicator * pow((minLine + l_s - l_i), 2) + s_i) + (1 - m_Conf.Photoacoustic)*s_i;
    }

    for (short l_s1 = minLine; l_s1 < maxLine - 1; ++l_s1)
    {
      if (AddSample[l_s1 - minLine] < (short)inputS && AddSample[l_s1 - minLine] >= 0)
      {
        for (short l_s2 = l_s1 + 1; l_s2 < maxLine; ++l_s2)
        {
          if (AddSample[l_s2 - minLine] < inputS && AddSample[l_s2 - minLine] >= 0)
          {
            mult = input[l_s2 + AddSample[l_s2 - minLine] * (short)inputL] * apodisation[(short)((l_s2 - minLine)*apod_mult)] * input[l_s1 + AddSample[l_s1 - minLine] * (short)inputL] * apodisation[(short)((l_s1 - minLine)*apod_mult)];
            output[sample*(short)outputL + line] += sqrt(abs(mult)) * ((mult > 0) - (mult < 0));
          }
        }
      }
      else
        --usedLines;
    }

    output[sample*(short)outputL + line] = 10 * output[sample*(short)outputL + line] / (pow(usedLines, 2) - (usedLines - 1));

    delete[] AddSample;
  }
}

void mitk::BeamformingFilter::DMASSphericalLine(float* input, float* output, float inputDim[2], float outputDim[2], const short& line, float* apodisation, const short& apodArraySize)
{
  float& inputS = inputDim[1];
  float& inputL = inputDim[0];

  float& outputS = outputDim[1];
  float& outputL = outputDim[0];

  short maxLine = 0;
  short minLine = 0;
  float l_i = 0;
  float s_i = 0;

  float part = 0.07 * inputL;
  float tan_phi = std::tan(m_Conf.Angle / 360 * 2 * M_PI);
  float part_multiplicator = tan_phi * m_Conf.TimeSpacing * m_Conf.SpeedOfSound / m_Conf.Pitch * m_Conf.ReconstructionLines / m_Conf.TransducerElements;
  float apod_mult = 1;

  float mult = 0;

  short usedLines = (maxLine - minLine);

  //exact delay

  l_i = line / outputL * inputL;

  for (short sample = 0; sample < outputS; ++sample)
  {
    s_i = sample / outputS * inputS / 2;

    part = part_multiplicator*s_i;

    if (part < 1)
      part = 1;

    maxLine = (short)std::min((l_i + part) + 1, inputL);
    minLine = (short)std::max((l_i - part), 0.0f);
    usedLines = (maxLine - minLine);

    apod_mult = apodArraySize / (maxLine - minLine);

    //calculate the AddSamples beforehand to save some time
    short* AddSample = new short[maxLine - minLine];
    for (short l_s = 0; l_s < maxLine - minLine; ++l_s)
    {
      AddSample[l_s] = (short)sqrt(
        pow(s_i, 2)
        +
        pow((1 / (m_Conf.TimeSpacing*m_Conf.SpeedOfSound) * ((minLine + l_s - l_i)*m_Conf.Pitch*m_Conf.TransducerElements) / inputL), 2)
      ) + (1 - m_Conf.Photoacoustic)*s_i;
    }

    for (short l_s1 = minLine; l_s1 < maxLine - 1; ++l_s1)
    {
      if (AddSample[l_s1 - minLine] < inputS && AddSample[l_s1 - minLine] >= 0)
      {
        for (short l_s2 = l_s1 + 1; l_s2 < maxLine; ++l_s2)
        {
          if (AddSample[l_s2 - minLine] < inputS && AddSample[l_s2 - minLine] >= 0)
          {
            mult = input[l_s2 + AddSample[l_s2 - minLine] * (short)inputL] * apodisation[(int)((l_s2 - minLine)*apod_mult)] * input[l_s1 + AddSample[l_s1 - minLine] * (short)inputL] * apodisation[(int)((l_s1 - minLine)*apod_mult)];
            output[sample*(short)outputL + line] += sqrt(abs(mult)) * ((mult > 0) - (mult < 0));
          }
        }
      }
      else
        --usedLines;
    }

    output[sample*(short)outputL + line] = 10 * output[sample*(short)outputL + line] / (pow(usedLines, 2) - (usedLines - 1));

    delete[] AddSample;
  }
}