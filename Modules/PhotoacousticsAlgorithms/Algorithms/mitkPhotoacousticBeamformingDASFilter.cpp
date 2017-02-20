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


#include "mitkPhotoacousticBeamformingDASFilter.h"
#include "mitkProperties.h"
#include "mitkImageReadAccessor.h"
#include <algorithm>
#include <itkImageIOBase.h>


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
  
  unsigned int dim[] = { m_Conf.ReconstructionLines, m_Conf.SamplesPerLine, input->GetDimension(2) };
  output->Initialize(mitk::MakeScalarPixelType<double>(), 3, dim);

  mitk::Vector3D spacing;
  spacing[0] = m_Conf.Pitch * m_Conf.TransducerElements * 1000 / m_Conf.ReconstructionLines;
  spacing[1] = m_Conf.RecordTime * m_Conf.SpeedOfSound / 2 * 1000 / m_Conf.SamplesPerLine;
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

  float inputH = input->GetDimension(1);
  float inputW = input->GetDimension(0);

  float outputH = output->GetDimension(1);
  float outputW = output->GetDimension(0);

  if (!output->IsInitialized())
  {
    return;
  }

  float part = 0.07 * inputW;

  for (int i = 0; i < input->GetDimension(2); ++i) // seperate Slices should get Beamforming seperately applied
  {
    mitk::ImageReadAccessor inputReadAccessor(input, input->GetSliceData(i));
    m_InputData = (double*)inputReadAccessor.GetData();
    m_OutputData = new double[m_Conf.ReconstructionLines*m_Conf.SamplesPerLine];

    for (int l = 0; l < outputW; ++l)
    {
      for (int s = 0; s < outputH; ++s)
      {
        m_OutputData[l*(unsigned short)outputH + s] = 0;
      }
    }

    int AddSample = 0;
    unsigned short maxLine = 0;
    unsigned short minLine = 0;
    float delayMultiplicator = 0;

    for (unsigned short line = 0; line < outputW; ++line)
    {
      float l_i = line / outputW * inputW;
      for (unsigned short sample = 0; sample < outputH; ++sample)
      {
        float s_i = sample / outputH * inputH;
        delayMultiplicator = ((inputW / 2 - l_i) / s_i);

        maxLine = (unsigned short)std::min((l_i + part)+1, inputW);
        minLine = (unsigned short)std::max((l_i - part), 0.0f);

        for (unsigned short l_s = minLine; l_s < maxLine; ++l_s)
        {
          AddSample = delayMultiplicator * l_s + s_i;
          if(AddSample < inputH && AddSample >=0)
            m_OutputData[sample*(unsigned short)outputW + line] += m_InputData[l_s+AddSample*(int)inputH];
        }
      }
    }

    output->SetSlice(m_OutputData, i);

    delete[] m_OutputData;
    m_OutputData = nullptr;
    m_InputData = nullptr;
  }
  m_TimeOfHeaderInitialization.Modified();
}

void mitk::BeamformingDASFilter::Configure(beamformingSettings settings)
{
  m_Conf = settings;
}