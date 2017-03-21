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

#include "mitkPhotoacousticBeamformingDMASFilter.h"
#include "mitkProperties.h"
#include "mitkImageReadAccessor.h"
#include <algorithm>
#include <itkImageIOBase.h>
#include <chrono>
#include "itkFFT1DComplexConjugateToRealImageFilter.h"
#include "itkFFT1DRealToComplexConjugateImageFilter.h"
#include "mitkImageCast.h"
#include <cmath>
#include <thread>

// needed itk image filters
#include "mitkITKImageImport.h"
#include "itkFFTShiftImageFilter.h"
#include "itkMultiplyImageFilter.h"
#include "itkComplexToModulusImageFilter.h"
#include <itkAddImageFilter.h>

mitk::BeamformingDMASFilter::BeamformingDMASFilter() : m_OutputData(nullptr), m_InputData(nullptr)
{
  this->SetNumberOfIndexedInputs(1);
  this->SetNumberOfRequiredInputs(1);

  m_Conf.Pitch = 0.0003;
  m_Conf.SpeedOfSound = 1540;
  m_Conf.SamplesPerLine = 2048;
  m_Conf.ReconstructionLines = 128;
  m_Conf.RecordTime = 0.00006;
  m_Conf.TransducerElements = 128;

  m_ProgressHandle = [](int) {};
}

void mitk::BeamformingDMASFilter::SetProgressHandle(std::function<void(int)> progressHandle)
{
  m_ProgressHandle = progressHandle;
}

mitk::BeamformingDMASFilter::~BeamformingDMASFilter()
{
}

void mitk::BeamformingDMASFilter::GenerateInputRequestedRegion()
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

void mitk::BeamformingDMASFilter::GenerateOutputInformation()
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
  spacing[1] = m_Conf.RecordTime * m_Conf.SpeedOfSound * 1000 / m_Conf.SamplesPerLine;
  spacing[2] = 1;

  output->GetGeometry()->SetSpacing(spacing);
  output->GetGeometry()->Modified();
  output->SetPropertyList(input->GetPropertyList()->Clone());

  m_TimeOfHeaderInitialization.Modified();
}

void mitk::BeamformingDMASFilter::GenerateData()
{
  mitk::Image::ConstPointer input = this->GetInput();
  mitk::Image::Pointer output = this->GetOutput();

  double inputDim[2] = { input->GetDimension(0), input->GetDimension(1) / ((int)m_Conf.Photoacoustic + 1) };
  double outputDim[2] = { output->GetDimension(0), output->GetDimension(1) };

  const int apodArraySize = m_Conf.TransducerElements * 4;
  double* VonHannWindow = VonHannFunction(apodArraySize);

  int progInterval = output->GetDimension(2) / 20 > 1 ? output->GetDimension(2) / 20 : 1;

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

    if (input->GetPixelType().GetTypeAsString() == "scalar (double)" || input->GetPixelType().GetTypeAsString() == " (double)")
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

    const unsigned short max_threads = 12;

    std::thread *threads = new std::thread[(unsigned short)outputDim[0]];

    for (unsigned short line = 0; line < outputDim[0]; ++line)
    {
      if (m_Conf.DelayCalculationMethod == beamformingSettings::DelayCalc::Linear)
      {
        threads[line] = std::thread(&BeamformingDMASFilter::DMASLinearLine, this, m_InputData, m_OutputData, inputDim, outputDim, line, VonHannWindow, apodArraySize);
      }
      else if (m_Conf.DelayCalculationMethod == beamformingSettings::DelayCalc::QuadApprox)
      {
        threads[line] = std::thread(&BeamformingDMASFilter::DMASQuadraticLine, this, m_InputData, m_OutputData, inputDim, outputDim, line, VonHannWindow, apodArraySize);
      }
      else if (m_Conf.DelayCalculationMethod == beamformingSettings::DelayCalc::Spherical)
      {
        threads[line] = std::thread(&BeamformingDMASFilter::DMASSphericalLine, this, m_InputData, m_OutputData, inputDim, outputDim, line, VonHannWindow, apodArraySize);
      }
    }
    for (unsigned short line = 0; line < outputDim[0]; ++line)
    {
      threads[line].join();
    }

    /*
    bool *threadfinished = new bool[max_threads];
    for (unsigned short r = 0; r < max_threads; ++r)
    {
      threadfinished[r] = false;
    }

    unsigned short line = 0;
    while(line < outputDim[0])
    {
      //DMASSphericalLine(m_InputData, m_OutputData, inputDim, outputDim, line, VonHannWindow, apodArraySize)
      //threads[line] = std::thread(&BeamformingDMASFilter::DMASSphericalLine, this, m_InputData, m_OutputData, inputDim, outputDim, line, VonHannWindow, apodArraySize);
      for (unsigned short n = 0; n < max_threads; ++n)
      {
        if (threadfinished[n])
        {
          threads[n].join();
          threadfinished[n] = false;
          //MITK_INFO << "thread " << n << " joined";
        }
        if (!threads[n].joinable())
        {
          threads[n] = std::thread(&BeamformingDMASFilter::DMASSphericalLine, this, m_InputData, m_OutputData, inputDim, outputDim, line, VonHannWindow, apodArraySize, &threadfinished[n]);
          ++line;
          //MITK_INFO << "thread " << n << " created for line " << line - 1;
          break;
        }
      }
    }
    for (unsigned short n = 0; n < max_threads; ++n)
    {
      if (threads[n].joinable())
      {
        threads[n].join();
        threadfinished[n] = false;
        //MITK_INFO << "thread " << n << " joined";
      }
    }
    delete[] threadfinished;*/
    //threadpool... seems slower

    output->SetSlice(m_OutputData, i);

    if (i % progInterval == 0)
      m_ProgressHandle((int)((i + 1) / (double)output->GetDimension(2) * 100));

    delete[] m_OutputData;
    delete[] m_InputDataPuffer;
    delete[] threads;
    m_OutputData = nullptr;
    m_InputData = nullptr;
  }

  if (m_Conf.UseBP)
  {
    mitk::Image::Pointer BP = BandpassFilter(output);

    for (int i = 0; i < output->GetDimension(2); ++i)
    {
      mitk::ImageReadAccessor copy(BP, BP->GetSliceData(i));
      output->SetSlice(copy.GetData(), i);
    }
  }

  m_TimeOfHeaderInitialization.Modified();

  delete[] VonHannWindow;

  auto end = std::chrono::high_resolution_clock::now();
  MITK_INFO << "DMAS Beamforming of " << output->GetDimension(2) << " Images completed in " << ((double)std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count()) / 1000000 << "ms" << std::endl;
}

void mitk::BeamformingDMASFilter::Configure(beamformingSettings settings)
{
  m_Conf = settings;
}

mitk::Image::Pointer mitk::BeamformingDMASFilter::BandpassFilter(mitk::Image::Pointer data)
{
  typedef double PixelType;
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

  double singleVoxel = 1 / (m_Conf.RecordTime / data->GetDimension(1)) / 2 / 1000;
  double BoundHighPass = std::min(m_Conf.BPHighPass / singleVoxel, (double)data->GetDimension(1) / 2);
  double BoundLowPass = std::min(m_Conf.BPLowPass / singleVoxel, (double)data->GetDimension(1) / 2 - BoundHighPass);

  int center1 = ((- BoundLowPass - BoundHighPass + data->GetDimension(1) / 2) / 2) + BoundLowPass;
  int center2 = ((- BoundLowPass - BoundHighPass + data->GetDimension(1) / 2) / 2) + BoundHighPass + data->GetDimension(1) / 2;

  int width1 = -BoundLowPass - BoundHighPass + data->GetDimension(1) / 2;
  int width2 = -BoundLowPass - BoundHighPass + data->GetDimension(1) / 2;

  
  /*MITK_INFO << "BHP " << BoundHighPass << " BLP " << BoundLowPass << "BPLP" << m_Conf.BPLowPass;
  MITK_INFO << "center1 " << center1 << " width1 " << width1;
  MITK_INFO << "center2 " << center2 << " width2 " << width2;*/ //debugging

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
  return GrabItkImageMemory(toReal->GetOutput()); */ //DEBUG

  typedef itk::FFT1DComplexConjugateToRealImageFilter< ComplexImageType, RealImageType > InverseFilterType;
  InverseFilterType::Pointer inverseFFTFilter = InverseFilterType::New();
  inverseFFTFilter->SetInput(multiplyFilter->GetOutput());
  inverseFFTFilter->SetDirection(1);

  return GrabItkImageMemory(inverseFFTFilter->GetOutput());
}

itk::Image<double, 3U>::Pointer mitk::BeamformingDMASFilter::BPFunction(mitk::Image::Pointer reference, int width, int center)
{
  // tukey window
  double alpha = m_Conf.BPFalloff;

  double* imageData = new double[reference->GetDimension(0)*reference->GetDimension(1)];

  for (int sample = 0; sample < reference->GetDimension(1); ++sample)
  {
    imageData[reference->GetDimension(0)*sample] = 0;
  }

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
  }

  for (int line = 1; line < reference->GetDimension(0); ++line)
  {
    for (int sample = 0; sample < reference->GetDimension(1); ++sample)
    {
      imageData[reference->GetDimension(0)*sample + line] = imageData[reference->GetDimension(0)*sample];
    }
  }

  typedef itk::Image< double, 3U >  ImageType;
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
  image->FillBuffer(itk::NumericTraits<double>::Zero);
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

double* mitk::BeamformingDMASFilter::VonHannFunction(int samples)
{
  double* VonHannWindow = new double[samples];

  for (int n = 0; n < samples; ++n)
  {
    VonHannWindow[n] = (1 - cos(2 * M_PI * n / (samples - 1))) / 2;
  }

  return VonHannWindow;
}

void mitk::BeamformingDMASFilter::DMASLinearLine(double* input, double* output, double inputDim[2], double outputDim[2], const unsigned short& line, double* apodisation, const unsigned short& apodArraySize)
{
  double& inputS = inputDim[1];
  double& inputL = inputDim[0];

  double& outputS = outputDim[1];
  double& outputL = outputDim[0];

  unsigned short AddSample1 = 0;
  unsigned short AddSample2 = 0;
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
  double part_multiplicator = tan_phi * m_Conf.RecordTime / inputS * m_Conf.SpeedOfSound / m_Conf.Pitch * m_Conf.ReconstructionLines / m_Conf.TransducerElements;
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

    //calculate the AddSamples beforehand to save some time
    unsigned short* AddSample = new unsigned short[maxLine - minLine];
    for (unsigned short l_s = 0; l_s < maxLine - minLine; ++l_s)
    {
      AddSample[l_s] = abs((unsigned short)(delayMultiplicator * (minLine + l_s - l_i) + s_i));
    }

    for (unsigned short l_s1 = minLine; l_s1 < maxLine - 1; ++l_s1)
    {
      if (AddSample[l_s1 - minLine] < inputS && AddSample[l_s1 - minLine] >= 0)
      {
        for (unsigned short l_s2 = l_s1 + 1; l_s2 < maxLine; ++l_s2)
        {
          if (AddSample[l_s2 - minLine] < inputS && AddSample[l_s2 - minLine] >= 0)
          {
            mult = input[l_s2 + AddSample[l_s2 - minLine] * (unsigned short)inputL] * input[l_s1 + AddSample[l_s1 - minLine] * (unsigned short)inputL] * apodisation[(unsigned short)((l_s1 - minLine)*VH_mult)] * apodisation[(unsigned short)((l_s2 - minLine)*VH_mult)];
            output[sample*(unsigned short)outputL + line] += sqrt(abs(mult)) * ((mult > 0) - (mult < 0));
          }
        }
      }
    }

    output[sample*(unsigned short)outputL + line] = 10 * output[sample*(unsigned short)outputL + line] / (pow(maxLine - minLine, 2) - (maxLine - minLine));

    delete[] AddSample;

  }
}

void mitk::BeamformingDMASFilter::DMASQuadraticLine(double* input, double* output, double inputDim[2], double outputDim[2], const unsigned short& line, double* apodisation, const unsigned short& apodArraySize)
{
  double& inputS = inputDim[1];
  double& inputL = inputDim[0];

  double& outputS = outputDim[1];
  double& outputL = outputDim[0];

  unsigned short AddSample1 = 0;
  unsigned short AddSample2 = 0;
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
  double part_multiplicator = tan_phi * m_Conf.RecordTime / inputS * m_Conf.SpeedOfSound / m_Conf.Pitch * m_Conf.ReconstructionLines / m_Conf.TransducerElements;
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

    //calculate the AddSamples beforehand to save some time
    unsigned short* AddSample = new unsigned short[maxLine - minLine];
    for (unsigned short l_s = 0; l_s < maxLine - minLine; ++l_s)
    {
      AddSample[l_s] = (unsigned short)(delayMultiplicator * pow((minLine + l_s - l_i), 2) + s_i);
    }

    for (unsigned short l_s1 = minLine; l_s1 < maxLine - 1; ++l_s1)
    {
      if (AddSample[l_s1 - minLine] < inputS && AddSample[l_s1 - minLine] >= 0)
      {
        for (unsigned short l_s2 = l_s1 + 1; l_s2 < maxLine; ++l_s2)
        {
          if (AddSample[l_s2 - minLine] < inputS && AddSample[l_s2 - minLine] >= 0)
          {
            mult = input[l_s2 + AddSample[l_s2 - minLine] * (unsigned short)inputL] * apodisation[(unsigned short)((l_s2 - minLine)*VH_mult)] * input[l_s1 + AddSample[l_s1 - minLine] * (unsigned short)inputL] * apodisation[(unsigned short)((l_s1 - minLine)*VH_mult)];
            output[sample*(unsigned short)outputL + line] += sqrt(abs(mult)) * ((mult > 0) - (mult < 0));
          }
        }
      }
    }

    output[sample*(unsigned short)outputL + line] = 10 * output[sample*(unsigned short)outputL + line] / (pow(maxLine - minLine, 2) - (maxLine - minLine - 1));


    delete[] AddSample;
  }
}

void mitk::BeamformingDMASFilter::DMASSphericalLine(double* input, double* output, double inputDim[2], double outputDim[2], const unsigned short& line, double* apodisation, const unsigned short& apodArraySize)
{
  double& inputS = inputDim[1];
  double& inputL = inputDim[0];

  double& outputS = outputDim[1];
  double& outputL = outputDim[0];

  unsigned short AddSample1 = 0;
  unsigned short AddSample2 = 0;
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
  double part_multiplicator = tan_phi * m_Conf.RecordTime / inputS * m_Conf.SpeedOfSound / m_Conf.Pitch * m_Conf.ReconstructionLines / m_Conf.TransducerElements;
  double VH_mult = 1;

  double mult = 0;

  //exact delay

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

    //calculate the AddSamples beforehand to save some time
    unsigned short* AddSample = new unsigned short[maxLine - minLine];
    for (unsigned short l_s = 0; l_s < maxLine - minLine; ++l_s)
    {
      AddSample[l_s] = (unsigned short)sqrt(
        pow(s_i, 2)
        +
        pow((inputS / (m_Conf.RecordTime*m_Conf.SpeedOfSound) * ((minLine + l_s - l_i)*m_Conf.Pitch*m_Conf.TransducerElements) / inputL), 2)
      );
    }

    for (unsigned short l_s1 = minLine; l_s1 < maxLine - 1; ++l_s1)
    {
      if (AddSample[l_s1 - minLine] < inputS && AddSample[l_s1 - minLine] >= 0)
      {
        for (unsigned short l_s2 = l_s1 + 1; l_s2 < maxLine; ++l_s2)
        {
          if (AddSample[l_s2 - minLine] < inputS && AddSample[l_s2 - minLine] >= 0)
          {
            mult = input[l_s2 + AddSample[l_s2 - minLine] * (unsigned short)inputL] * apodisation[(int)((l_s2 - minLine)*VH_mult)] * input[l_s1 + AddSample[l_s1 - minLine] * (unsigned short)inputL] * apodisation[(int)((l_s1 - minLine)*VH_mult)];
            output[sample*(unsigned short)outputL + line] += sqrt(abs(mult)) * ((mult > 0) - (mult < 0));
          }
        }
      }
    }

    output[sample*(unsigned short)outputL + line] = 10 * output[sample*(unsigned short)outputL + line] / (pow(maxLine - minLine, 2) - (maxLine - minLine - 1));

    delete[] AddSample;
  }
}