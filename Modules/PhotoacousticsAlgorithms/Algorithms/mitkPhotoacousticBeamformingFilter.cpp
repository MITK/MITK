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
#include "itkFFT1DComplexConjugateToRealImageFilter.h"
#include "itkFFT1DRealToComplexConjugateImageFilter.h"
#include "mitkImageCast.h"
#include <mitkPhotoacousticOCLBeamformer.h>

// needed itk image filters
#include "mitkITKImageImport.h"
#include "itkFFTShiftImageFilter.h"
#include "itkMultiplyImageFilter.h"
#include "itkComplexToModulusImageFilter.h"
#include <itkAddImageFilter.h>


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
  spacing[1] = m_Conf.RecordTime * m_Conf.SpeedOfSound * 1000 / m_Conf.SamplesPerLine;
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

  float inputDim[2] = { input->GetDimension(0), input->GetDimension(1) / ((int)m_Conf.Photoacoustic + 1) };
  // if the photoacoustic option is used, we halve the image, as only the upper part of it contains any information
  float outputDim[2] = { output->GetDimension(0), output->GetDimension(1) };
  unsigned int oclOutputDim[3] = { output->GetDimension(0), output->GetDimension(1), output->GetDimension(2) };

  unsigned short chunkSize = 40;
  unsigned int oclOutputDimChunk[3] = { output->GetDimension(0), output->GetDimension(1), chunkSize < input->GetDimension(2) ? chunkSize : input->GetDimension(2) };
  unsigned int oclInputDimChunk[3] = { input->GetDimension(0), input->GetDimension(1), chunkSize < input->GetDimension(2) ? chunkSize : input->GetDimension(2) };
  

  const int apodArraySize = m_Conf.TransducerElements * 4; // set the resolution of the apodization array
  float* ApodWindow;
  // calculate the appropiate apodization window
  if (m_Conf.Apod == beamformingSettings::Apodization::Hann)
  {
    ApodWindow = VonHannFunction(apodArraySize);
  }
  else if (m_Conf.Apod == beamformingSettings::Apodization::Hamm)
  {
    ApodWindow = HammFunction(apodArraySize);
  }
  else
  {
    ApodWindow = BoxFunction(apodArraySize);
  }

  int progInterval = output->GetDimension(2) / 20 > 1 ? output->GetDimension(2) / 20 : 1;
  // the interval at which we update the gui progress bar

  auto begin = std::chrono::high_resolution_clock::now(); // debbuging the performance...
  if (!m_Conf.UseGPU)
  {
    for (int i = 0; i < output->GetDimension(2); ++i) // seperate Slices should get Beamforming seperately applied
    {
      mitk::ImageReadAccessor inputReadAccessor(input, input->GetSliceData(i));

      m_OutputData = new float[m_Conf.ReconstructionLines*m_Conf.SamplesPerLine];
      m_InputDataPuffer = new float[input->GetDimension(0)*input->GetDimension(1)];

      // first, we convert any data to float, which we use by default
      if (input->GetPixelType().GetTypeAsString() == "scalar (float)" || input->GetPixelType().GetTypeAsString() == " (float)")
      {
        m_InputData = (float*)inputReadAccessor.GetData();
      }
      else
      {
        MITK_INFO << "Pixel type is not float, abort";
        return;
      }

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
      delete[] m_InputDataPuffer;
      m_OutputData = nullptr;
      m_InputData = nullptr;
    }
  }
  else
  {
    mitk::PhotoacousticOCLBeamformer::Pointer m_oclFilter = mitk::PhotoacousticOCLBeamformer::New();
    us::ServiceReference<OclResourceService> ref = GetModuleContext()->GetServiceReference<OclResourceService>();
    OclResourceService* resources = GetModuleContext()->GetService<OclResourceService>(ref);
    resources->GetContext();

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
      m_oclFilter->SetBeamformingParameters(m_Conf.SpeedOfSound, m_Conf.RecordTime, m_Conf.Pitch, m_Conf.Angle, m_Conf.Photoacoustic, m_Conf.TransducerElements);

      for (int i = 0; i < ceil((float)oclOutputDim[2] / (float)chunkSize); ++i)
      {
        mitk::Image::Pointer chunk = mitk::Image::New();
        chunk->Initialize(input->GetPixelType(), 3, oclInputDimChunk);
        chunk->SetSpacing(input->GetGeometry()->GetSpacing());

        mitk::ImageReadAccessor ChunkMove(input);
        chunk->SetImportVolume((void*)&(((float*)const_cast<void*>(ChunkMove.GetData()))[i * chunkSize * input->GetDimension(0) * input->GetDimension(1)]), 0, 0, mitk::Image::ReferenceMemory);
        
        m_oclFilter->SetInput(chunk);
        m_oclFilter->Update();

        auto out = m_oclFilter->GetOutput();
        for (int s = i * chunkSize; s < oclOutputDim[2]; ++s)
        {
          mitk::ImageReadAccessor copy(out, out->GetSliceData(s - i * chunkSize));
          output->SetImportSlice(const_cast<void*>(copy.GetData()), s, 0, 0, mitk::Image::ReferenceMemory);
        }
      }
    }
    catch (mitk::Exception &e)
    {
      std::string errorMessage = "Caught unexpected exception ";
      errorMessage.append(e.what());
      MITK_ERROR << errorMessage;
    }
  }

  // apply a bandpass filter, if requested
  if (m_Conf.UseBP)
  {
    m_ProgressHandle(100, "applying bandpass");
    mitk::Image::Pointer BP = BandpassFilter(output);

    mitk::ImageReadAccessor copy(BP);
    output->SetImportVolume(const_cast<void*>(copy.GetData()), 0, 0, mitk::Image::ReferenceMemory);
  }

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

  float l = 0;
  float x = 0;
  float root = 0;

  float part = 0.07 * inputL;
  float tan_phi = std::tan(m_Conf.Angle / 360 * 2 * M_PI);
  float part_multiplicator = tan_phi * m_Conf.RecordTime / inputS * m_Conf.SpeedOfSound / m_Conf.Pitch * m_Conf.ReconstructionLines / m_Conf.TransducerElements;
  float apod_mult = 1;

  float mult = 0;
  short usedLines = (maxLine - minLine);

  //quadratic delay
  l_i = line / outputL * inputL;

  for (short sample = 0; sample < outputS; ++sample)
  {
    s_i = sample / outputS * inputS;

    part = part_multiplicator*s_i;

    if (part < 1)
      part = 1;

    maxLine = (short)std::min((l_i + part) + 1, inputL);
    minLine = (short)std::max((l_i - part), 0.0f);
    usedLines = (maxLine - minLine);

    apod_mult = apodArraySize / (maxLine - minLine);

    delayMultiplicator = pow((inputS / (m_Conf.RecordTime*m_Conf.SpeedOfSound) * (m_Conf.Pitch*m_Conf.TransducerElements) / inputL), 2) / s_i / 2;

    for (short l_s = minLine; l_s < maxLine; ++l_s)
    {
      AddSample = delayMultiplicator * pow((l_s - l_i), 2) + s_i;
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
  float delayMultiplicator = 0;
  float l_i = 0;
  float s_i = 0;

  float part = 0.07 * inputL;
  float tan_phi = std::tan(m_Conf.Angle / 360 * 2 * M_PI);
  float part_multiplicator = tan_phi * m_Conf.RecordTime / inputS * m_Conf.SpeedOfSound / m_Conf.Pitch * m_Conf.ReconstructionLines / m_Conf.TransducerElements;
  float apod_mult = 1;

  short usedLines = (maxLine - minLine);

  //exact delay

  l_i = (float)line / outputL * inputL;

  for (short sample = 0; sample < outputS; ++sample)
  {
    s_i = (float)sample / outputS * inputS;

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
        pow((inputS / (m_Conf.RecordTime*m_Conf.SpeedOfSound) * ((l_s - l_i)*m_Conf.Pitch*m_Conf.TransducerElements) / inputL), 2)
      );
      if (AddSample < inputS && AddSample >= 0) 
        output[sample*(short)outputL + line] += input[l_s + AddSample*(short)inputL] * apodisation[(short)((l_s - minLine)*apod_mult)];
      else
        --usedLines;
    }
    output[sample*(short)outputL + line] = output[sample*(short)outputL + line] / usedLines;
  }
}

mitk::Image::Pointer mitk::BeamformingFilter::BandpassFilter(mitk::Image::Pointer data)
{
  // do a fourier transform, multiply with an appropriate window for the filter, and transform back
  typedef float PixelType;
  typedef itk::Image< PixelType, 3 > RealImageType;
  RealImageType::Pointer image;

  mitk::CastToItkImage(data, image);

  typedef itk::FFT1DRealToComplexConjugateImageFilter<RealImageType> ForwardFFTFilterType;
  typedef ForwardFFTFilterType::OutputImageType ComplexImageType;
  ForwardFFTFilterType::Pointer forwardFFTFilter = ForwardFFTFilterType::New();
  forwardFFTFilter->SetInput(image);
  forwardFFTFilter->SetDirection(1);
  try
  {
    forwardFFTFilter->UpdateOutputInformation();
  }
  catch (itk::ExceptionObject & error)
  {
    std::cerr << "Error: " << error << std::endl;
    MITK_WARN << "Bandpass can not be applied after beamforming";
    return data;
  }

  float singleVoxel = 1 / (m_Conf.RecordTime / data->GetDimension(1)) / 2 / 1000;
  float BoundHighPass = std::min(m_Conf.BPHighPass / singleVoxel, (float)data->GetDimension(1) / 2);
  float BoundLowPass = std::min(m_Conf.BPLowPass / singleVoxel, (float)data->GetDimension(1) / 2 - BoundHighPass);

  int center1 = ((-BoundLowPass - BoundHighPass + data->GetDimension(1) / 2) / 2) + BoundLowPass;
  int center2 = ((-BoundLowPass - BoundHighPass + data->GetDimension(1) / 2) / 2) + BoundHighPass + data->GetDimension(1) / 2;

  int width1 = -BoundLowPass - BoundHighPass + data->GetDimension(1) / 2;
  int width2 = -BoundLowPass - BoundHighPass + data->GetDimension(1) / 2;

  /*MITK_INFO << "BHP " << BoundHighPass << " BLP " << BoundLowPass << "BPLP" << m_Conf.BPLowPass;
  MITK_INFO << "center1 " << center1 << " width1 " << width1;
  MITK_INFO << "center2 " << center2 << " width2 " << width2;*/ //DEBUG

  RealImageType::Pointer fftMultiplicator1 = BPFunction(data, width1, center1);
  RealImageType::Pointer fftMultiplicator2 = BPFunction(data, width2, center2);

  typedef itk::AddImageFilter<RealImageType, RealImageType> AddImageFilterType;
  AddImageFilterType::Pointer addImageFilter = AddImageFilterType::New();
  addImageFilter->SetInput1(fftMultiplicator1);
  addImageFilter->SetInput2(fftMultiplicator2);

  typedef itk::FFTShiftImageFilter< RealImageType, RealImageType > FFTShiftFilterType;
  FFTShiftFilterType::Pointer fftShiftFilter = FFTShiftFilterType::New();
  fftShiftFilter->SetInput(addImageFilter->GetOutput());

  typedef itk::MultiplyImageFilter< ComplexImageType,
    RealImageType,
    ComplexImageType >
    MultiplyFilterType;
  MultiplyFilterType::Pointer multiplyFilter = MultiplyFilterType::New();
  multiplyFilter->SetInput1(forwardFFTFilter->GetOutput());
  multiplyFilter->SetInput2(fftShiftFilter->GetOutput());

  /*itk::ComplexToModulusImageFilter<ComplexImageType, RealImageType>::Pointer toReal = itk::ComplexToModulusImageFilter<ComplexImageType, RealImageType>::New();
  toReal->SetInput(multiplyFilter->GetOutput());
  return GrabItkImageMemory(addImageFilter->GetOutput());*/  //DEBUG

  typedef itk::FFT1DComplexConjugateToRealImageFilter< ComplexImageType, RealImageType > InverseFilterType;
  InverseFilterType::Pointer inverseFFTFilter = InverseFilterType::New();
  inverseFFTFilter->SetInput(multiplyFilter->GetOutput());
  inverseFFTFilter->SetDirection(1);

  return GrabItkImageMemory(inverseFFTFilter->GetOutput());
}

itk::Image<float, 3U>::Pointer mitk::BeamformingFilter::BPFunction(mitk::Image::Pointer reference, int width, int center)
{
  float* imageData = new float[reference->GetDimension(0)*reference->GetDimension(1)];

  for (int sample = 0; sample < reference->GetDimension(1); ++sample)
  {
    imageData[reference->GetDimension(0)*sample] = 0;
  }

  /* // tukey window
  float alpha = m_Conf.BPFalloff;

  for (int n = 0; n < width; ++n)
  {
  if (n <= (alpha*(width - 1)) / 2)
  {
  imageData[reference->GetDimension(0)*(n + center - (int)(width / 2))] = (1 + cos(M_PI*(2 * n / (alpha*(width - 1)) - 1))) / 2;
  }
  else if (n >= (width - 1)*(1 - alpha / 2) && n <= (width - 1))
  {
  imageData[reference->GetDimension(0)*(n + center - (int)(width / 2))] = (1 + cos(M_PI*(2 * n / (alpha*(width - 1)) + 1 - 2 / alpha))) / 2;
  }
  else
  {
  imageData[reference->GetDimension(0)*(n + center - (int)(width / 2))] = 1;
  }
  }*/

  // Butterworth-Filter
  float d = center - width / 2;
  float l = center + width / 2;

  for (int n = 0; n < reference->GetDimension(1); ++n)
  {
    imageData[reference->GetDimension(0)*n] = 1 / (1 + pow(((float)center - (float)n) / ((float)width / 2), 2 * m_Conf.ButterworthOrder));
  }

  // copy and paste to all lines
  for (int line = 1; line < reference->GetDimension(0); ++line)
  {
    for (int sample = 0; sample < reference->GetDimension(1); ++sample)
    {
      imageData[reference->GetDimension(0)*sample + line] = imageData[reference->GetDimension(0)*sample];
    }
  }

  typedef itk::Image< float, 3U >  ImageType;
  ImageType::RegionType region;
  ImageType::IndexType start;
  start.Fill(0);

  region.SetIndex(start);

  ImageType::SizeType size;
  size[0] = reference->GetDimension(0);
  size[1] = reference->GetDimension(1);
  size[2] = reference->GetDimension(2);

  region.SetSize(size);

  ImageType::SpacingType SpacingItk;
  SpacingItk[0] = reference->GetGeometry()->GetSpacing()[0];
  SpacingItk[1] = reference->GetGeometry()->GetSpacing()[1];
  SpacingItk[2] = reference->GetGeometry()->GetSpacing()[2];

  ImageType::Pointer image = ImageType::New();
  image->SetRegions(region);
  image->Allocate();
  image->FillBuffer(itk::NumericTraits<float>::Zero);
  image->SetSpacing(SpacingItk);

  ImageType::IndexType pixelIndex;

  for (ImageType::IndexValueType slice = 0; slice < reference->GetDimension(2); ++slice)
  {
    for (ImageType::IndexValueType line = 0; line < reference->GetDimension(0); ++line)
    {
      for (ImageType::IndexValueType sample = 0; sample < reference->GetDimension(1); ++sample)
      {
        pixelIndex[0] = line;
        pixelIndex[1] = sample;
        pixelIndex[2] = slice;

        image->SetPixel(pixelIndex, imageData[line + sample*reference->GetDimension(0)]);
      }
    }
  }

  delete[] imageData;

  return image;
}

void mitk::BeamformingFilter::DMASQuadraticLine(float* input, float* output, float inputDim[2], float outputDim[2], const short& line, float* apodisation, const short& apodArraySize)
{
  float& inputS = inputDim[1];
  float& inputL = inputDim[0];

  float& outputS = outputDim[1];
  float& outputL = outputDim[0];

  short AddSample1 = 0;
  short AddSample2 = 0;
  short maxLine = 0;
  short minLine = 0;
  float delayMultiplicator = 0;
  float l_i = 0;
  float s_i = 0;

  float l = 0;
  float x = 0;
  float root = 0;

  float part = 0.07 * inputL;
  float tan_phi = std::tan(m_Conf.Angle / 360 * 2 * M_PI);
  float part_multiplicator = tan_phi * m_Conf.RecordTime / inputS * m_Conf.SpeedOfSound / m_Conf.Pitch * m_Conf.ReconstructionLines / m_Conf.TransducerElements;
  float apod_mult = 1;

  float mult = 0;
  short usedLines = (maxLine - minLine);

  //quadratic delay
  l_i = line / outputL * inputL;

  for (short sample = 0; sample < outputS; ++sample)
  {
    s_i = sample / outputS * inputS;

    part = part_multiplicator*s_i;

    if (part < 1)
      part = 1;

    maxLine = (short)std::min((l_i + part) + 1, inputL);
    minLine = (short)std::max((l_i - part), 0.0f);
    usedLines = (maxLine - minLine);

    apod_mult = apodArraySize / (maxLine - minLine);

    delayMultiplicator = pow((inputS / (m_Conf.RecordTime*m_Conf.SpeedOfSound) * (m_Conf.Pitch*m_Conf.TransducerElements) / inputL), 2) / s_i / 2;

    //calculate the AddSamples beforehand to save some time
    short* AddSample = new short[maxLine - minLine];
    for (short l_s = 0; l_s < maxLine - minLine; ++l_s)
    {
      AddSample[l_s] = (short)(delayMultiplicator * pow((minLine + l_s - l_i), 2) + s_i);
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

  short AddSample1 = 0;
  short AddSample2 = 0;
  short maxLine = 0;
  short minLine = 0;
  float delayMultiplicator = 0;
  float l_i = 0;
  float s_i = 0;

  float l = 0;
  float x = 0;
  float root = 0;

  float part = 0.07 * inputL;
  float tan_phi = std::tan(m_Conf.Angle / 360 * 2 * M_PI);
  float part_multiplicator = tan_phi * m_Conf.RecordTime / inputS * m_Conf.SpeedOfSound / m_Conf.Pitch * m_Conf.ReconstructionLines / m_Conf.TransducerElements;
  float apod_mult = 1;

  float mult = 0;

  short usedLines = (maxLine - minLine);

  //exact delay

  l_i = line / outputL * inputL;

  for (short sample = 0; sample < outputS; ++sample)
  {
    s_i = sample / outputS * inputS;

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
        pow((inputS / (m_Conf.RecordTime*m_Conf.SpeedOfSound) * ((minLine + l_s - l_i)*m_Conf.Pitch*m_Conf.TransducerElements) / inputL), 2)
      );
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