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

#define _USE_MATH_DEFINES

#include "mitkPhotoacousticBeamformingDASFilter.h"
#include "mitkProperties.h"
#include "mitkImageReadAccessor.h"
#include <algorithm>
#include <itkImageIOBase.h>
#include <chrono>
#include <cmath>
#include <thread>


mitk::BeamformingDASFilter::BeamformingDASFilter() : m_OutputData(nullptr), m_InputData(nullptr)
{
  this->SetNumberOfIndexedInputs(1);
  this->SetNumberOfRequiredInputs(1);

  m_Conf.Pitch = 0.0003;
  m_Conf.SpeedOfSound = 1540;
  m_Conf.SamplesPerLine = 2048;
  m_Conf.ReconstructionLines = 128;
  m_Conf.RecordTime = 0.00006;
  m_Conf.TransducerElements = 128;
}

mitk::BeamformingDASFilter::~BeamformingDASFilter()
{
}

void mitk::BeamformingDASFilter::GenerateInputRequestedRegion()
{
  Superclass::GenerateInputRequestedRegion();

  mitk::Image* output = this->GetOutput();
  mitk::Image* input = const_cast< mitk::Image * > (this->GetInput());
  if (!output->IsInitialized())
  {
    return;
  }

  input->SetRequestedRegionToLargestPossibleRegion();

  //GenerateTimeInInputRegion(output, input);
}

void mitk::BeamformingDASFilter::GenerateOutputInformation()
{
  mitk::Image::ConstPointer input = this->GetInput();
  mitk::Image::Pointer output = this->GetOutput();

  if ((output->IsInitialized()) && (this->GetMTime() <= m_TimeOfHeaderInitialization.GetMTime()))
    return;

  itkDebugMacro(<< "GenerateOutputInformation()");
  
  unsigned int dim[] = { m_Conf.ReconstructionLines, m_Conf.SamplesPerLine, input->GetDimension(2)};
  output->Initialize(mitk::MakeScalarPixelType<double>(), 3, dim);

  mitk::Vector3D spacing;
  spacing[0] = m_Conf.Pitch * m_Conf.TransducerElements * 1000 / m_Conf.ReconstructionLines;
  spacing[1] = m_Conf.RecordTime * m_Conf.SpeedOfSound * 1000 / m_Conf.SamplesPerLine;
  spacing[2] = 1;

  output->GetGeometry()->SetSpacing(spacing);
  output->GetGeometry()->Modified();
  output->SetPropertyList(input->GetPropertyList()->Clone());

  m_TimeOfHeaderInitialization.Modified();
}

void mitk::BeamformingDASFilter::GenerateData()
{
  mitk::Image::ConstPointer input = this->GetInput();
  mitk::Image::Pointer output = this->GetOutput();

  double inputDim[2] = { input->GetDimension(0), input->GetDimension(1) / ((int)m_Conf.Photoacoustic + 1) };
  double outputDim[2] = { output->GetDimension(0), output->GetDimension(1) };

  const int apodArraySize = m_Conf.TransducerElements * 4;
  double* VonHannWindow = VonHannFunction(apodArraySize);

  if (!output->IsInitialized())
  {
    return;
  }

  auto begin = std::chrono::high_resolution_clock::now(); // debbuging the performance...

  for (int i = 0; i < output->GetDimension(2); ++i) // seperate Slices should get Beamforming seperately applied
  {
    mitk::ImageReadAccessor inputReadAccessor(input, input->GetSliceData(i));

    m_OutputData = new double[m_Conf.ReconstructionLines*m_Conf.SamplesPerLine];
    m_InputDataPuffer = new double[input->GetDimension(0)*input->GetDimension(1)];

    if (input->GetPixelType().GetTypeAsString() == "scalar (double)")
    {
      m_InputData = (double*)inputReadAccessor.GetData();
    }
    else if (input->GetPixelType().GetTypeAsString() == "scalar (short)")
    {
      short* InputPuffer = (short*)inputReadAccessor.GetData();
      for (int l = 0; l < inputDim[0]; ++l)
      {
        for (int s = 0; s < inputDim[1]; ++s)
        {
          m_InputDataPuffer[l*(unsigned short)inputDim[1] + s] = (double)InputPuffer[l*(unsigned short)inputDim[1] + s];
        }
      }
      m_InputData = m_InputDataPuffer;
    }
    else if (input->GetPixelType().GetTypeAsString() == "scalar (float)")
    {
      float* InputPuffer = (float*)inputReadAccessor.GetData();
      for (int l = 0; l < inputDim[0]; ++l)
      {
        for (int s = 0; s < inputDim[1]; ++s)
        {
          m_InputDataPuffer[l*(unsigned short)inputDim[1] + s] = (double)InputPuffer[l*(unsigned short)inputDim[1] + s];
        }
      }
      m_InputData = m_InputDataPuffer;
    }
    else
    {
      MITK_INFO << "Could not determine pixel type";
      return;
    }

    for (int l = 0; l < outputDim[0]; ++l)
    {
      for (int s = 0; s < outputDim[1]; ++s)
      {
        m_OutputData[l*(unsigned short)outputDim[1] + s] = 0;
      }
    }

    std::thread *threads = new std::thread[(unsigned short)outputDim[0]];

    if (m_Conf.DelayCalculationMethod == beamformingSettings::DelayCalc::Linear)
    {
      //linear delay
      for (unsigned short line = 0; line < outputDim[0]; ++line)
      {
        //DMASLinearLine(m_InputData, m_OutputData, inputDim, outputDim, line, VonHannWindow, apodArraySize)
        threads[line] = std::thread(&BeamformingDASFilter::DASLinearLine, this, m_InputData, m_OutputData, inputDim, outputDim, line, VonHannWindow, apodArraySize);
      }
      for (unsigned short line = 0; line < outputDim[0]; ++line)
      {
        threads[line].join();
      }
    }
    else if (m_Conf.DelayCalculationMethod == beamformingSettings::DelayCalc::QuadApprox)
    {
      //quadratic delay
      for (unsigned short line = 0; line < outputDim[0]; ++line)
      {
        //DMASQuadraticLine(m_InputData, m_OutputData, inputDim, outputDim, line, VonHannWindow, apodArraySize)
        threads[line] = std::thread(&BeamformingDASFilter::DASQuadraticLine, this, m_InputData, m_OutputData, inputDim, outputDim, line, VonHannWindow, apodArraySize);
      }
      for (unsigned short line = 0; line < outputDim[0]; ++line)
      {
        threads[line].join();
      }
    }
    else if (m_Conf.DelayCalculationMethod == beamformingSettings::DelayCalc::Spherical)
    {
      //exact delay
      for (unsigned short line = 0; line < outputDim[0]; ++line)
      {
        threads[line] = std::thread(&BeamformingDASFilter::DASSphericalLine, this, m_InputData, m_OutputData, inputDim, outputDim, line, VonHannWindow, apodArraySize);
      }
      for (unsigned short line = 0; line < outputDim[0]; ++line)
      {
        threads[line].join();
      }
    }

    output->SetSlice(m_OutputData, i);

    if (i % 50 == 0)
      MITK_INFO << "slice " << i + 1 << " of " << output->GetDimension(2) << " computed";

    delete[] m_OutputData;
    delete[] m_InputDataPuffer;
    m_OutputData = nullptr;
    m_InputData = nullptr;
  }
  m_TimeOfHeaderInitialization.Modified();

  auto end = std::chrono::high_resolution_clock::now();
  MITK_INFO << "DAS Beamforming of " << output->GetDimension(2) << " Images completed in " << ((double)std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count()) / 1000000 << "ms" << std::endl;
}

void mitk::BeamformingDASFilter::Configure(beamformingSettings settings)
{
  m_Conf = settings;
}

double* mitk::BeamformingDASFilter::VonHannFunction(int samples)
{
  double* VonHannWindow = new double[samples];

  for (int n = 0; n < samples; ++n)
  {
    VonHannWindow[n] = (1 - cos(2 * M_PI * n / (samples - 1))) / 2;
  }

  return VonHannWindow;
}


void mitk::BeamformingDASFilter::DASLinearLine(double* input, double* output, double inputDim[2], double outputDim[2], const unsigned short& line, double* apodisation, const unsigned short& apodArraySize)
{
  double& inputS = inputDim[1];
  double& inputL = inputDim[0];

  double& outputS = outputDim[1];
  double& outputL = outputDim[0];

  unsigned short AddSample = 0;
  unsigned short maxLine = 0;
  unsigned short minLine = 0;
  double delayMultiplicator = 0;
  double l_i = 0;
  double s_i = 0;

  double l = 0;
  double x = 0;
  double root = 0;

  double part = 0.07 * inputL;
  double tan_phi = std::tan(m_Conf.Angle / 360 * 2 * M_PI);
  double part_multiplicator = tan_phi * m_Conf.RecordTime / inputS * m_Conf.SpeedOfSound / m_Conf.Pitch;
  double VH_mult = 1;

  double mult = 0;

  //linear delay
  l_i = line / outputL * inputL;

  l = (inputL / 2 - l_i) / inputL*m_Conf.Pitch*m_Conf.TransducerElements;

  for (unsigned short sample = 0; sample < outputS; ++sample)
  {
    s_i = sample / outputS * inputS;

    part = part_multiplicator*s_i;

    if (part < 1)
      part = 1;

    maxLine = (unsigned short)std::min((l_i + part) + 1, inputL);
    minLine = (unsigned short)std::max((l_i - part), 0.0);
    VH_mult = apodArraySize / (maxLine - minLine);

    x = m_Conf.RecordTime / inputS * s_i * m_Conf.SpeedOfSound;
    root = l / sqrt(pow(l, 2) + pow(m_Conf.RecordTime / inputS * s_i * m_Conf.SpeedOfSound, 2));
    delayMultiplicator = root / (m_Conf.RecordTime*m_Conf.SpeedOfSound) *m_Conf.Pitch*m_Conf.TransducerElements / inputL;

    for (unsigned short l_s = minLine; l_s < maxLine; ++l_s)
    {
      AddSample = delayMultiplicator * (l_s - l_i) + s_i;
      if (AddSample < inputS && AddSample >= 0)
        output[sample*(unsigned short)outputL + line] += input[l_s + AddSample*(unsigned short)inputL] * apodisation[(unsigned short)((l_s - minLine)*VH_mult)];
    }
    output[sample*(unsigned short)outputL + line] = output[sample*(unsigned short)outputL + line] / (maxLine - minLine);
  }
}

void mitk::BeamformingDASFilter::DASQuadraticLine(double* input, double* output, double inputDim[2], double outputDim[2], const unsigned short& line, double* apodisation, const unsigned short& apodArraySize)
{
  double& inputS = inputDim[1];
  double& inputL = inputDim[0];

  double& outputS = outputDim[1];
  double& outputL = outputDim[0];

  unsigned short AddSample = 0;
  unsigned short maxLine = 0;
  unsigned short minLine = 0;
  double delayMultiplicator = 0;
  double l_i = 0;
  double s_i = 0;

  double l = 0;
  double x = 0;
  double root = 0;

  double part = 0.07 * inputL;
  double tan_phi = std::tan(m_Conf.Angle / 360 * 2 * M_PI);
  double part_multiplicator = tan_phi * m_Conf.RecordTime / inputS * m_Conf.SpeedOfSound / m_Conf.Pitch;
  double VH_mult = 1;

  double mult = 0;

  //quadratic delay
  l_i = line / outputL * inputL;

  for (unsigned short sample = 0; sample < outputS; ++sample)
  {
    s_i = sample / outputS * inputS;

    part = part_multiplicator*s_i;

    if (part < 1)
      part = 1;

    maxLine = (unsigned short)std::min((l_i + part) + 1, inputL);
    minLine = (unsigned short)std::max((l_i - part), 0.0);
    VH_mult = apodArraySize / (maxLine - minLine);

    delayMultiplicator = pow((inputS / (m_Conf.RecordTime*m_Conf.SpeedOfSound) * (m_Conf.Pitch*m_Conf.TransducerElements) / inputL), 2) / s_i / 2;

    for (unsigned short l_s = minLine; l_s < maxLine; ++l_s)
    {
      AddSample = delayMultiplicator * pow((l_s - l_i), 2) + s_i;
      if (AddSample < inputS && AddSample >= 0) {
        output[sample*(unsigned short)outputL + line] += input[l_s + AddSample*(unsigned short)inputL] * apodisation[(unsigned short)((l_s - minLine)*VH_mult)];
      }
    }
    output[sample*(unsigned short)outputL + line] = output[sample*(unsigned short)outputL + line] / (maxLine - minLine);
  }
}

void mitk::BeamformingDASFilter::DASSphericalLine(double* input, double* output, double inputDim[2], double outputDim[2], const unsigned short& line, double* apodisation, const unsigned short& apodArraySize)
{
  double& inputS = inputDim[1];
  double& inputL = inputDim[0];

  double& outputS = outputDim[1];
  double& outputL = outputDim[0];

  unsigned short AddSample = 0;
  unsigned short maxLine = 0;
  unsigned short minLine = 0;
  double delayMultiplicator = 0;
  double l_i = 0;
  double s_i = 0;

  double l = 0;
  double x = 0;
  double root = 0;

  double part = 0.07 * inputL;
  double tan_phi = std::tan(m_Conf.Angle / 360 * 2 * M_PI);
  double part_multiplicator = tan_phi * m_Conf.RecordTime / inputS * m_Conf.SpeedOfSound / m_Conf.Pitch;
  double VH_mult = 1;

  double mult = 0;

  //exact delay

  l_i = (double)line / outputL * inputL;

  for (unsigned short sample = 0; sample < outputS; ++sample)
  {
    s_i = (double)sample / outputS * inputS;

    part = part_multiplicator*s_i;

    if (part < 1)
      part = 1;

    maxLine = (unsigned short)std::min((l_i + part) + 1, inputL);
    minLine = (unsigned short)std::max((l_i - part), 0.0);
    VH_mult = apodArraySize / (maxLine - minLine);

    for (unsigned short l_s = minLine; l_s < maxLine; ++l_s)
    {
      AddSample = (int)sqrt(
        pow(s_i, 2)
        +
        pow((inputS / (m_Conf.RecordTime*m_Conf.SpeedOfSound) * ((l_s - l_i)*m_Conf.Pitch*m_Conf.TransducerElements) / inputL), 2)
      );
      if (AddSample < inputS && AddSample >= 0) {
        output[sample*(unsigned short)outputL + line] += input[l_s + AddSample*(unsigned short)inputL] * apodisation[(unsigned short)((l_s - minLine)*VH_mult)];
      }
    }
    output[sample*(unsigned short)outputL + line] = output[sample*(unsigned short)outputL + line] / (maxLine - minLine);
  }
}