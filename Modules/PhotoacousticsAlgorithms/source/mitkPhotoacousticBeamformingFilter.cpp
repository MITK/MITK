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
#include "mitkPhotoacousticBeamformingFilter.h"

mitk::BeamformingFilter::BeamformingFilter() : m_OutputData(nullptr), m_InputData(nullptr), m_Message("noMessage")
{
  this->SetNumberOfIndexedInputs(1);
  this->SetNumberOfRequiredInputs(1);

  m_ProgressHandle = [](int, std::string) {};

  m_BeamformingOclFilter = mitk::PhotoacousticOCLBeamformingFilter::New();

  m_VonHannFunction = VonHannFunction(m_Conf.apodizationArraySize);
  m_HammFunction = HammFunction(m_Conf.apodizationArraySize);
  m_BoxFunction = BoxFunction(m_Conf.apodizationArraySize);
}

void mitk::BeamformingFilter::SetProgressHandle(std::function<void(int, std::string)> progressHandle)
{
  m_ProgressHandle = progressHandle;
}

mitk::BeamformingFilter::~BeamformingFilter()
{
  delete[] m_VonHannFunction;
  delete[] m_HammFunction;
  delete[] m_BoxFunction;
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
  spacing[1] = (m_Conf.TimeSpacing * m_Conf.inputDim[1]) / 2 * m_Conf.SpeedOfSound * 1000 / m_Conf.SamplesPerLine;
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

  float* ApodWindow;
  if (m_ConfOld.apodizationArraySize != m_Conf.apodizationArraySize)
  {
    delete[] m_VonHannFunction;
    delete[] m_HammFunction;
    delete[] m_BoxFunction;
    m_VonHannFunction = VonHannFunction(m_Conf.apodizationArraySize);
    m_HammFunction = HammFunction(m_Conf.apodizationArraySize);
    m_BoxFunction = BoxFunction(m_Conf.apodizationArraySize);

    m_ConfOld = m_Conf;
  }

  // set the appropiate apodization window
  switch (m_Conf.Apod)
  {
  case BeamformingSettings::Apodization::Hann:
    ApodWindow = m_VonHannFunction;
    break;
  case BeamformingSettings::Apodization::Hamm:
    ApodWindow = m_HammFunction;
    break;
  case BeamformingSettings::Apodization::Box:
    ApodWindow = m_BoxFunction;
    break;
  default:
    ApodWindow = m_BoxFunction;
    break;
  }

  auto begin = std::chrono::high_resolution_clock::now(); // debbuging the performance...

  if (!m_Conf.UseGPU)
  {
    int progInterval = output->GetDimension(2) / 20 > 1 ? output->GetDimension(2) / 20 : 1;
    // the interval at which we update the gui progress bar

    float inputDim[2] = { (float)input->GetDimension(0), (float)input->GetDimension(1) };
    float outputDim[2] = { (float)output->GetDimension(0), (float)output->GetDimension(1) };

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
      if (m_Conf.Algorithm == BeamformingSettings::BeamformingAlgorithm::DAS)
      {
        if (m_Conf.DelayCalculationMethod == BeamformingSettings::DelayCalc::QuadApprox)
        {
          for (short line = 0; line < outputDim[0]; ++line)
          {
            threads[line] = std::thread(&BeamformingFilter::DASQuadraticLine, this, m_InputData, m_OutputData, inputDim, outputDim, line, ApodWindow, m_Conf.apodizationArraySize);
          }
        }
        else if (m_Conf.DelayCalculationMethod == BeamformingSettings::DelayCalc::Spherical)
        {
          for (short line = 0; line < outputDim[0]; ++line)
          {
            threads[line] = std::thread(&BeamformingFilter::DASSphericalLine, this, m_InputData, m_OutputData, inputDim, outputDim, line, ApodWindow, m_Conf.apodizationArraySize);
          }
        }
      }
      else if (m_Conf.Algorithm == BeamformingSettings::BeamformingAlgorithm::DMAS)
      {
        if (m_Conf.DelayCalculationMethod == BeamformingSettings::DelayCalc::QuadApprox)
        {
          for (short line = 0; line < outputDim[0]; ++line)
          {
            threads[line] = std::thread(&BeamformingFilter::DMASQuadraticLine, this, m_InputData, m_OutputData, inputDim, outputDim, line, ApodWindow, m_Conf.apodizationArraySize);
          }
        }
        else if (m_Conf.DelayCalculationMethod == BeamformingSettings::DelayCalc::Spherical)
        {
          for (short line = 0; line < outputDim[0]; ++line)
          {
            threads[line] = std::thread(&BeamformingFilter::DMASSphericalLine, this, m_InputData, m_OutputData, inputDim, outputDim, line, ApodWindow, m_Conf.apodizationArraySize);
          }
        }
      }
      else if (m_Conf.Algorithm == BeamformingSettings::BeamformingAlgorithm::sDMAS)
      {
        if (m_Conf.DelayCalculationMethod == BeamformingSettings::DelayCalc::QuadApprox)
        {
          for (short line = 0; line < outputDim[0]; ++line)
          {
            threads[line] = std::thread(&BeamformingFilter::sDMASQuadraticLine, this, m_InputData, m_OutputData, inputDim, outputDim, line, ApodWindow, m_Conf.apodizationArraySize);
          }
        }
        else if (m_Conf.DelayCalculationMethod == BeamformingSettings::DelayCalc::Spherical)
        {
          for (short line = 0; line < outputDim[0]; ++line)
          {
            threads[line] = std::thread(&BeamformingFilter::sDMASSphericalLine, this, m_InputData, m_OutputData, inputDim, outputDim, line, ApodWindow, m_Conf.apodizationArraySize);
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
  #if defined(PHOTOACOUSTICS_USE_GPU) || DOXYGEN
  else
  {
    try
    {
      // first, we check whether the data is float, other formats are unsupported
      if (!(input->GetPixelType().GetTypeAsString() == "scalar (float)" || input->GetPixelType().GetTypeAsString() == " (float)"))
      {
        MITK_ERROR << "Pixel type is not float, abort";
        return;
      }

      unsigned long availableMemory = m_BeamformingOclFilter->GetDeviceMemory();

      unsigned int batchSize = 16;
      unsigned int batches = (unsigned int)((float)input->GetDimension(2)/batchSize) + (input->GetDimension(2)%batchSize > 0);

      unsigned int batchDim[] = { input->GetDimension(0), input->GetDimension(1), batchSize };
      unsigned int batchDimLast[] = { input->GetDimension(0), input->GetDimension(1), input->GetDimension(2) % batchSize };

      // the following safeguard is probably only needed for absurdly small GPU memory
      for (batchSize = 16; 
        (unsigned long)batchSize *
        ((unsigned long)(batchDim[0] * batchDim[1]) * 4 + // single input image (float)
        (unsigned long)(m_Conf.ReconstructionLines * m_Conf.SamplesPerLine) * 4) // single output image (float)
        > availableMemory -
        (unsigned long)(m_Conf.ReconstructionLines / 2 * m_Conf.SamplesPerLine) * 2 - // Delays buffer (unsigned short)
        (unsigned long)(m_Conf.ReconstructionLines * m_Conf.SamplesPerLine) * 3 * 2 - // UsedLines buffer (unsigned short)
        50 * 1024 * 1024; // 50 MB buffer for local data, system purposes etc
        --batchSize)
      {}
      if (batchSize < 1)
      {
        MITK_ERROR << "device memory too small for GPU beamforming";
        return;
      }

      mitk::ImageReadAccessor copy(input);

      for(unsigned int i = 0; i < batches; ++i)
      {
        m_ProgressHandle(input->GetDimension(2)/batches * i, "performing reconstruction");

        mitk::Image::Pointer inputBatch = mitk::Image::New();
        if(i == batches - 1 && (input->GetDimension(2)%batchSize > 0))
        {
          inputBatch->Initialize(mitk::MakeScalarPixelType<float>(), 3, batchDimLast);
          m_Conf.inputDim[2] = batchDimLast[2];
        }
        else
        {
          inputBatch->Initialize(mitk::MakeScalarPixelType<float>(), 3, batchDim);
          m_Conf.inputDim[2] = batchDim[2];
        }

        inputBatch->SetSpacing(input->GetGeometry()->GetSpacing());

        inputBatch->SetImportVolume(&(((float*)copy.GetData())[input->GetDimension(0) * input->GetDimension(1) * batchSize * i]));

        m_BeamformingOclFilter->SetApodisation(ApodWindow, m_Conf.apodizationArraySize);
        m_BeamformingOclFilter->SetConfig(m_Conf);
        m_BeamformingOclFilter->SetInput(inputBatch);
        m_BeamformingOclFilter->Update();

        void* out = m_BeamformingOclFilter->GetOutput();

        for(unsigned int slice = 0; slice < m_Conf.inputDim[2]; ++slice)
        {
          output->SetImportSlice(
                &(((float*)out)[m_Conf.ReconstructionLines * m_Conf.SamplesPerLine * slice]),
              batchSize * i + slice, 0, 0, mitk::Image::ImportMemoryManagementType::CopyMemory);
        }
      }
    }
    catch (mitk::Exception &e)
    {
      std::string errorMessage = "Caught unexpected exception ";
      errorMessage.append(e.what());
      MITK_ERROR << errorMessage;

      float* dummyData = new float[m_Conf.ReconstructionLines * m_Conf.SamplesPerLine * m_Conf.inputDim[2]];
      output->SetImportVolume(dummyData, 0, 0, mitk::Image::ImportMemoryManagementType::ManageMemory);

      m_Message = "An openCL error occurred; all GPU operations in this and the next session may be corrupted.";
    }
  }
  #endif
  m_TimeOfHeaderInitialization.Modified();

  auto end = std::chrono::high_resolution_clock::now();
  MITK_INFO << "Beamforming of " << output->GetDimension(2) << " Images completed in " << ((float)std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count()) / 1000000 << "ms" << std::endl;
}

float* mitk::BeamformingFilter::VonHannFunction(int samples)
{
  float* ApodWindow = new float[samples];

  for (int n = 0; n < samples; ++n)
  {
    ApodWindow[n] = (1 - cos(2 * itk::Math::pi * n / (samples - 1))) / 2;
  }

  return ApodWindow;
}

float* mitk::BeamformingFilter::HammFunction(int samples)
{
  float* ApodWindow = new float[samples];

  for (int n = 0; n < samples; ++n)
  {
    ApodWindow[n] = 0.54 - 0.46*cos(2 * itk::Math::pi*n / (samples - 1));
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
  float tan_phi = std::tan(m_Conf.Angle / 360 * 2 * itk::Math::pi);
  float part_multiplicator = tan_phi * m_Conf.TimeSpacing * m_Conf.SpeedOfSound / m_Conf.Pitch * inputL / m_Conf.TransducerElements;
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

    apod_mult = (float)apodArraySize / (float)usedLines;

    delayMultiplicator = pow((1 / (m_Conf.TimeSpacing*m_Conf.SpeedOfSound) * (m_Conf.Pitch*m_Conf.TransducerElements) / inputL), 2) / s_i / 2;

    for (short l_s = minLine; l_s < maxLine; ++l_s)
    {
      AddSample = delayMultiplicator * pow((l_s - l_i), 2) + s_i + (1 - m_Conf.isPhotoacousticImage)*s_i;
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
  float tan_phi = std::tan(m_Conf.Angle / 360 * 2 * itk::Math::pi);
  float part_multiplicator = tan_phi * m_Conf.TimeSpacing * m_Conf.SpeedOfSound / m_Conf.Pitch * inputL / (float)m_Conf.TransducerElements;
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

    apod_mult = (float)apodArraySize / (float)usedLines;

    for (short l_s = minLine; l_s < maxLine; ++l_s)
    {
      AddSample = (int)sqrt(
        pow(s_i, 2)
        +
        pow((1 / (m_Conf.TimeSpacing*m_Conf.SpeedOfSound) * (((float)l_s - l_i)*m_Conf.Pitch*(float)m_Conf.TransducerElements) / inputL), 2)
      ) + (1 - m_Conf.isPhotoacousticImage)*s_i;
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
  float tan_phi = std::tan(m_Conf.Angle / 360 * 2 * itk::Math::pi);
  float part_multiplicator = tan_phi * m_Conf.TimeSpacing * m_Conf.SpeedOfSound / m_Conf.Pitch * inputL / (float)m_Conf.TransducerElements;
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

    apod_mult = (float)apodArraySize / (float)usedLines;

    delayMultiplicator = pow((1 / (m_Conf.TimeSpacing*m_Conf.SpeedOfSound) * (m_Conf.Pitch*m_Conf.TransducerElements) / inputL), 2) / s_i / 2;

    //calculate the AddSamples beforehand to save some time
    short* AddSample = new short[maxLine - minLine];
    for (short l_s = 0; l_s < maxLine - minLine; ++l_s)
    {
      AddSample[l_s] = (short)(delayMultiplicator * pow((minLine + l_s - l_i), 2) + s_i) + (1 - m_Conf.isPhotoacousticImage)*s_i;
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
  float tan_phi = std::tan(m_Conf.Angle / 360 * 2 * itk::Math::pi);
  float part_multiplicator = tan_phi * m_Conf.TimeSpacing * m_Conf.SpeedOfSound / m_Conf.Pitch * inputL / (float)m_Conf.TransducerElements;
  float apod_mult = 1;

  float mult = 0;

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

    apod_mult = (float)apodArraySize / (float)usedLines;

    //calculate the AddSamples beforehand to save some time
    short* AddSample = new short[maxLine - minLine];
    for (short l_s = 0; l_s < maxLine - minLine; ++l_s)
    {
      AddSample[l_s] = (short)sqrt(
        pow(s_i, 2)
        +
        pow((1 / (m_Conf.TimeSpacing*m_Conf.SpeedOfSound) * (((float)minLine + (float)l_s - l_i)*m_Conf.Pitch*(float)m_Conf.TransducerElements) / inputL), 2)
      ) + (1 - m_Conf.isPhotoacousticImage)*s_i;
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

void mitk::BeamformingFilter::sDMASQuadraticLine(float* input, float* output, float inputDim[2], float outputDim[2], const short& line, float* apodisation, const short& apodArraySize)
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
  float tan_phi = std::tan(m_Conf.Angle / 360 * 2 * itk::Math::pi);
  float part_multiplicator = tan_phi * m_Conf.TimeSpacing * m_Conf.SpeedOfSound / m_Conf.Pitch * inputL / (float)m_Conf.TransducerElements;
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

    apod_mult = (float)apodArraySize / (float)usedLines;

    delayMultiplicator = pow((1 / (m_Conf.TimeSpacing*m_Conf.SpeedOfSound) * (m_Conf.Pitch*m_Conf.TransducerElements) / inputL), 2) / s_i / 2;

    //calculate the AddSamples beforehand to save some time
    short* AddSample = new short[maxLine - minLine];
    for (short l_s = 0; l_s < maxLine - minLine; ++l_s)
    {
      AddSample[l_s] = (short)(delayMultiplicator * pow((minLine + l_s - l_i), 2) + s_i) + (1 - m_Conf.isPhotoacousticImage)*s_i;
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

void mitk::BeamformingFilter::sDMASSphericalLine(float* input, float* output, float inputDim[2], float outputDim[2], const short& line, float* apodisation, const short& apodArraySize)
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
  float tan_phi = std::tan(m_Conf.Angle / 360 * 2 * itk::Math::pi);
  float part_multiplicator = tan_phi * m_Conf.TimeSpacing * m_Conf.SpeedOfSound / m_Conf.Pitch * inputL / (float)m_Conf.TransducerElements;
  float apod_mult = 1;

  float mult = 0;

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

    apod_mult = (float)apodArraySize / (float)usedLines;

    //calculate the AddSamples beforehand to save some time
    short* AddSample = new short[maxLine - minLine];
    for (short l_s = 0; l_s < maxLine - minLine; ++l_s)
    {
      AddSample[l_s] = (short)sqrt(
        pow(s_i, 2)
        +
        pow((1 / (m_Conf.TimeSpacing*m_Conf.SpeedOfSound) * (((float)minLine + (float)l_s - l_i)*m_Conf.Pitch*(float)m_Conf.TransducerElements) / inputL), 2)
      ) + (1 - m_Conf.isPhotoacousticImage)*s_i;
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
