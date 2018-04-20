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
#include "mitkBeamformingFilter.h"
#include "mitkBeamformingUtils.h"

mitk::BeamformingFilter::BeamformingFilter(mitk::BeamformingSettings::Pointer settings) :
  m_OutputData(nullptr),
  m_InputData(nullptr),
  m_Message("noMessage"),
  m_Conf(settings)
{
  MITK_INFO << "Instantiating BeamformingFilter...";
  this->SetNumberOfIndexedInputs(1);
  this->SetNumberOfRequiredInputs(1);

  m_ProgressHandle = [](int, std::string) {};

  m_BeamformingOclFilter = mitk::PhotoacousticOCLBeamformingFilter::New(m_Conf);

  MITK_INFO << "Instantiating BeamformingFilter...[Done]";
}

void mitk::BeamformingFilter::SetProgressHandle(std::function<void(int, std::string)> progressHandle)
{
  m_ProgressHandle = progressHandle;
}

mitk::BeamformingFilter::~BeamformingFilter()
{
  MITK_INFO << "Destructed BeamformingFilter";
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
}

void mitk::BeamformingFilter::GenerateOutputInformation()
{
  mitk::Image::ConstPointer input = this->GetInput();
  mitk::Image::Pointer output = this->GetOutput();

  if ((output->IsInitialized()) && (this->GetMTime() <= m_TimeOfHeaderInitialization.GetMTime()))
    return;

  unsigned int dim[] = { m_Conf->GetReconstructionLines(), m_Conf->GetSamplesPerLine(), input->GetDimension(2) };
  output->Initialize(mitk::MakeScalarPixelType<float>(), 3, dim);

  mitk::Vector3D spacing;
  spacing[0] = m_Conf->GetPitchInMeters() * m_Conf->GetTransducerElements() * 1000 / m_Conf->GetReconstructionLines();
  spacing[1] = (m_Conf->GetTimeSpacing() * m_Conf->GetInputDim()[1]) / 2 * m_Conf->GetSpeedOfSound() * 1000 / m_Conf->GetSamplesPerLine();
  spacing[2] = 1;

  output->GetGeometry()->SetSpacing(spacing);
  output->GetGeometry()->Modified();
  output->SetPropertyList(input->GetPropertyList()->Clone());

  m_TimeOfHeaderInitialization.Modified();
}

void mitk::BeamformingFilter::GenerateData()
{
  mitk::Image::Pointer input = this->GetInput();
  if (!(input->GetPixelType().GetTypeAsString() == "scalar (float)" || input->GetPixelType().GetTypeAsString() == " (float)"))
  {
    mitkThrow() << "Pixel type of input needs to be float for this filter to work.";
  }

  GenerateOutputInformation();
  mitk::Image::Pointer output = this->GetOutput();

  if (!output->IsInitialized())
    return;

  auto begin = std::chrono::high_resolution_clock::now(); // debbuging the performance...

  if (!m_Conf->GetUseGPU())
  {
    int progInterval = output->GetDimension(2) / 20 > 1 ? output->GetDimension(2) / 20 : 1;
    // the interval at which we update the gui progress bar

    float inputDim[2] = { (float)input->GetDimension(0), (float)input->GetDimension(1) };
    float outputDim[2] = { (float)output->GetDimension(0), (float)output->GetDimension(1) };

    for (unsigned int i = 0; i < output->GetDimension(2); ++i) // seperate Slices should get Beamforming seperately applied
    {
      mitk::ImageReadAccessor inputReadAccessor(input, input->GetSliceData(i));
      m_InputData = (float*)inputReadAccessor.GetData();

      m_OutputData = new float[m_Conf->GetReconstructionLines()*m_Conf->GetSamplesPerLine()];

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
      if (m_Conf->GetAlgorithm() == BeamformingSettings::BeamformingAlgorithm::DAS)
      {
        if (m_Conf->GetDelayCalculationMethod() == BeamformingSettings::DelayCalc::QuadApprox)
        {
          for (short line = 0; line < outputDim[0]; ++line)
          {
            threads[line] = std::thread(&BeamformingUtils::DASQuadraticLine, m_InputData,
              m_OutputData, inputDim, outputDim, line, m_Conf->GetApodizationFunction(),
              m_Conf->GetApodizationArraySize(), m_Conf);
          }
        }
        else if (m_Conf->GetDelayCalculationMethod() == BeamformingSettings::DelayCalc::Spherical)
        {
          for (short line = 0; line < outputDim[0]; ++line)
          {
            threads[line] = std::thread(&BeamformingUtils::DASSphericalLine, m_InputData,
              m_OutputData, inputDim, outputDim, line, m_Conf->GetApodizationFunction(),
              m_Conf->GetApodizationArraySize(), m_Conf);
          }
        }
      }
      else if (m_Conf->GetAlgorithm() == BeamformingSettings::BeamformingAlgorithm::DMAS)
      {
        if (m_Conf->GetDelayCalculationMethod() == BeamformingSettings::DelayCalc::QuadApprox)
        {
          for (short line = 0; line < outputDim[0]; ++line)
          {
            threads[line] = std::thread(&BeamformingUtils::DMASQuadraticLine, m_InputData,
              m_OutputData, inputDim, outputDim, line, m_Conf->GetApodizationFunction(),
              m_Conf->GetApodizationArraySize(), m_Conf);
          }
        }
        else if (m_Conf->GetDelayCalculationMethod() == BeamformingSettings::DelayCalc::Spherical)
        {
          for (short line = 0; line < outputDim[0]; ++line)
          {
            threads[line] = std::thread(&BeamformingUtils::DMASSphericalLine, m_InputData,
              m_OutputData, inputDim, outputDim, line, m_Conf->GetApodizationFunction(),
              m_Conf->GetApodizationArraySize(), m_Conf);
          }
        }
      }
      else if (m_Conf->GetAlgorithm() == BeamformingSettings::BeamformingAlgorithm::sDMAS)
      {
        if (m_Conf->GetDelayCalculationMethod() == BeamformingSettings::DelayCalc::QuadApprox)
        {
          for (short line = 0; line < outputDim[0]; ++line)
          {
            threads[line] = std::thread(&BeamformingUtils::sDMASQuadraticLine, m_InputData,
              m_OutputData, inputDim, outputDim, line, m_Conf->GetApodizationFunction(),
              m_Conf->GetApodizationArraySize(), m_Conf);
          }
        }
        else if (m_Conf->GetDelayCalculationMethod() == BeamformingSettings::DelayCalc::Spherical)
        {
          for (short line = 0; line < outputDim[0]; ++line)
          {
            threads[line] = std::thread(&BeamformingUtils::sDMASSphericalLine, m_InputData,
              m_OutputData, inputDim, outputDim, line, m_Conf->GetApodizationFunction(),
              m_Conf->GetApodizationArraySize(), m_Conf);
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
      unsigned int batches = (unsigned int)((float)input->GetDimension(2) / batchSize) + (input->GetDimension(2) % batchSize > 0);

      unsigned int batchDim[] = { input->GetDimension(0), input->GetDimension(1), batchSize };
      unsigned int batchDimLast[] = { input->GetDimension(0), input->GetDimension(1), input->GetDimension(2) % batchSize };

      // the following safeguard is probably only needed for absurdly small GPU memory
      for (batchSize = 16;
        (unsigned long)batchSize *
        ((unsigned long)(batchDim[0] * batchDim[1]) * 4 + // single input image (float)
        (unsigned long)(m_Conf->GetReconstructionLines() * m_Conf->GetSamplesPerLine()) * 4) // single output image (float)
          > availableMemory -
        (unsigned long)(m_Conf->GetReconstructionLines() / 2 * m_Conf->GetSamplesPerLine()) * 2 - // Delays buffer (unsigned short)
        (unsigned long)(m_Conf->GetReconstructionLines() * m_Conf->GetSamplesPerLine()) * 3 * 2 - // UsedLines buffer (unsigned short)
        50 * 1024 * 1024; // 50 MB buffer for local data, system purposes etc
        --batchSize)
      {
      }
      if (batchSize < 1)
      {
        MITK_ERROR << "device memory too small for GPU beamforming";
        return;
      }

      mitk::ImageReadAccessor copy(input);

      for (unsigned int i = 0; i < batches; ++i)
      {
        m_ProgressHandle(input->GetDimension(2) / batches * i, "performing reconstruction");

        mitk::Image::Pointer inputBatch = mitk::Image::New();
        if (i == batches - 1 && (input->GetDimension(2) % batchSize > 0))
        {
          inputBatch->Initialize(mitk::MakeScalarPixelType<float>(), 3, batchDimLast);
          m_Conf->GetInputDim()[2] = batchDimLast[2];
        }
        else
        {
          inputBatch->Initialize(mitk::MakeScalarPixelType<float>(), 3, batchDim);
          m_Conf->GetInputDim()[2] = batchDim[2];
        }

        inputBatch->SetSpacing(input->GetGeometry()->GetSpacing());

        inputBatch->SetImportVolume(&(((float*)copy.GetData())[input->GetDimension(0) * input->GetDimension(1) * batchSize * i]));

        m_BeamformingOclFilter->SetApodisation(m_Conf->GetApodizationFunction(), m_Conf->GetApodizationArraySize());
        m_BeamformingOclFilter->SetInput(inputBatch);
        m_BeamformingOclFilter->Update();

        void* out = m_BeamformingOclFilter->GetOutput();

        for (unsigned int slice = 0; slice < m_Conf->GetInputDim()[2]; ++slice)
        {
          output->SetImportSlice(
            &(((float*)out)[m_Conf->GetReconstructionLines() * m_Conf->GetSamplesPerLine() * slice]),
            batchSize * i + slice, 0, 0, mitk::Image::ImportMemoryManagementType::CopyMemory);
        }
      }
    }
    catch (mitk::Exception &e)
    {
      std::string errorMessage = "Caught unexpected exception ";
      errorMessage.append(e.what());
      MITK_ERROR << errorMessage;

      float* dummyData = new float[m_Conf->GetReconstructionLines() * m_Conf->GetSamplesPerLine() * m_Conf->GetInputDim()[2]];
      output->SetImportVolume(dummyData, 0, 0, mitk::Image::ImportMemoryManagementType::ManageMemory);

      m_Message = "An openCL error occurred; all GPU operations in this and the next session may be corrupted.";
    }
  }
#endif
  m_TimeOfHeaderInitialization.Modified();

  auto end = std::chrono::high_resolution_clock::now();
  MITK_INFO << "Beamforming of " << output->GetDimension(2) << " Images completed in " << ((float)std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count()) / 1000000 << "ms" << std::endl;
}
