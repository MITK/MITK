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
#include <itkImageIOBase.h>
#include "itkFFT1DComplexConjugateToRealImageFilter.h"
#include "itkFFT1DRealToComplexConjugateImageFilter.h"
#include "mitkImageCast.h"

// needed itk image filters
#include "mitkITKImageImport.h"
#include "itkFFTShiftImageFilter.h"
#include "itkMultiplyImageFilter.h"
#include "itkComplexToModulusImageFilter.h"
#include <itkAddImageFilter.h>


mitk::BeamformingDASFilter::BeamformingDASFilter() : m_OutputData(nullptr), m_InputData(nullptr)
{
  this->SetNumberOfIndexedInputs(1);
  this->SetNumberOfRequiredInputs(1);

  m_ProgressHandle = [](int, std::string) {};
}

void mitk::BeamformingDASFilter::SetProgressHandle(std::function<void(int, std::string)> progressHandle)
{
  m_ProgressHandle = progressHandle;
}

mitk::BeamformingDASFilter::~BeamformingDASFilter()
{
}

void mitk::BeamformingDASFilter::GenerateInputRequestedRegion()
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
  spacing[1] = m_Conf.RecordTime * m_Conf.SpeedOfSound * 1000 / m_Conf.SamplesPerLine;
  spacing[2] = 1;

  output->GetGeometry()->SetSpacing(spacing);
  output->GetGeometry()->Modified();
  output->SetPropertyList(input->GetPropertyList()->Clone());

  m_TimeOfHeaderInitialization.Modified();
}

void mitk::BeamformingDASFilter::GenerateData()
{
  GenerateOutputInformation();
  mitk::Image::ConstPointer input = this->GetInput();
  mitk::Image::Pointer output = this->GetOutput();

  if (!output->IsInitialized())
    return;

  double inputDim[2] = { input->GetDimension(0), input->GetDimension(1) / ((int)m_Conf.Photoacoustic + 1) };
  // if the photoacoustic option is used, we halve the image, as only the upper part of it contains any information
  double outputDim[2] = { output->GetDimension(0), output->GetDimension(1) };

  const int apodArraySize = m_Conf.TransducerElements * 4; // set the resolution of the apodization array
  double* ApodWindow;
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

  for (int i = 0; i < output->GetDimension(2); ++i) // seperate Slices should get Beamforming seperately applied
  {
    mitk::ImageReadAccessor inputReadAccessor(input, input->GetSliceData(i));

    m_OutputData = new double[m_Conf.ReconstructionLines*m_Conf.SamplesPerLine];
    m_InputDataPuffer = new double[input->GetDimension(0)*input->GetDimension(1)];

    // first, we convert any data to double, which we use by default
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
          m_InputDataPuffer[l*(short)inputDim[1] + s] = (double)InputPuffer[l*(short)inputDim[1] + s];
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
          m_InputDataPuffer[l*(short)inputDim[1] + s] = (double)InputPuffer[l*(short)inputDim[1] + s];
        }
      }
      m_InputData = m_InputDataPuffer;
    }
    else
    {
      MITK_INFO << "Could not determine pixel type";
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
    for (short line = 0; line < outputDim[0]; ++line)
    {
      if (m_Conf.DelayCalculationMethod == beamformingSettings::DelayCalc::Linear)
      {
        threads[line] = std::thread(&BeamformingDASFilter::DASLinearLine, this, m_InputData, m_OutputData, inputDim, outputDim, line, ApodWindow, apodArraySize);
      }
      else if (m_Conf.DelayCalculationMethod == beamformingSettings::DelayCalc::QuadApprox)
      {
        threads[line] = std::thread(&BeamformingDASFilter::DASQuadraticLine, this, m_InputData, m_OutputData, inputDim, outputDim, line, ApodWindow, apodArraySize);
      }
      else if (m_Conf.DelayCalculationMethod == beamformingSettings::DelayCalc::Spherical)
      {
        threads[line] = std::thread(&BeamformingDASFilter::DASSphericalLine, this, m_InputData, m_OutputData, inputDim, outputDim, line, ApodWindow, apodArraySize);
      }
    }
    // wait for all lines to finish
    for (short line = 0; line < outputDim[0]; ++line)
    {
      threads[line].join();
    }

    output->SetSlice(m_OutputData, i);

    if (i % progInterval == 0)
      m_ProgressHandle((int)((i + 1) / (double)output->GetDimension(2) * 100), "performing reconstruction");

    delete[] m_OutputData;
    delete[] m_InputDataPuffer;
    m_OutputData = nullptr;
    m_InputData = nullptr;
  }

  // apply a bandpass filter, if requested
  if (m_Conf.UseBP)
  {
    m_ProgressHandle(100, "applying bandpass");
    mitk::Image::Pointer BP = BandpassFilter(output);

    for (int i = 0; i < output->GetDimension(2); ++i)
    {
      mitk::ImageReadAccessor copy(BP, BP->GetSliceData(i));
      output->SetSlice(copy.GetData(), i);
    }
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
  double* ApodWindow = new double[samples];

  for (int n = 0; n < samples; ++n)
  {
    ApodWindow[n] = (1 - cos(2 * M_PI * n / (samples - 1))) / 2;
  }

  return ApodWindow;
}

double* mitk::BeamformingDASFilter::HammFunction(int samples)
{
  double* ApodWindow = new double[samples];

  for (int n = 0; n < samples; ++n)
  {
    ApodWindow[n] = 0.54 - 0.46*cos(2 * M_PI*n / (samples - 1));
  }

  return ApodWindow;
}

double* mitk::BeamformingDASFilter::BoxFunction(int samples)
{
  double* ApodWindow = new double[samples];

  for (int n = 0; n < samples; ++n)
  {
    ApodWindow[n] = 1;
  }

  return ApodWindow;
}


void mitk::BeamformingDASFilter::DASLinearLine(double* input, double* output, double inputDim[2], double outputDim[2], const short& line, double* apodisation, const short& apodArraySize)
{
  double& inputS = inputDim[1];
  double& inputL = inputDim[0];

  double& outputS = outputDim[1];
  double& outputL = outputDim[0];

  short AddSample = 0;
  short maxLine = 0;
  short minLine = 0;
  double delayMultiplicator = 0;
  double l_i = 0;
  double s_i = 0;

  double l = 0;
  double x = 0;
  double root = 0;

  double part = 0.07 * inputL;
  double tan_phi = abs(std::tan(m_Conf.Angle / 360 * 2 * M_PI));
  double part_multiplicator = tan_phi * m_Conf.RecordTime / inputS * m_Conf.SpeedOfSound / m_Conf.Pitch * m_Conf.ReconstructionLines / m_Conf.TransducerElements;
  double apod_mult = 1;

  double mult = 0;
  short usedLines = (maxLine - minLine);

  //linear delay
  l_i = line / outputL * inputL;

  l = (inputL / 2 - l_i) / inputL * m_Conf.Pitch * m_Conf.TransducerElements;

  for (short sample = 0; sample < outputS; ++sample)
  {
    s_i = sample / outputS * inputS;

    part = part_multiplicator*s_i;

    if (part < 1)
      part = 1;

    maxLine = (short)std::min((l_i + part) + 1, inputL);
    minLine = (short)std::max((l_i - part), 0.0);
    usedLines = (maxLine - minLine);

    apod_mult = apodArraySize / (maxLine - minLine);

    x = m_Conf.RecordTime * m_Conf.SpeedOfSound * s_i / inputS;
    root = l / sqrt(pow(l, 2) + pow(x, 2));
    delayMultiplicator = inputS * root / (m_Conf.RecordTime * m_Conf.SpeedOfSound) * m_Conf.Pitch * m_Conf.TransducerElements / inputL;

    for (short l_s = minLine; l_s < maxLine; ++l_s)
    {
      AddSample = abs(delayMultiplicator * ((double)l_s - l_i)) + s_i;

      if (AddSample < inputS && AddSample >= 0)
        output[sample*(short)outputL + line] += input[l_s + AddSample*(short)inputL] * apodisation[(short)((l_s - minLine)*apod_mult)];
      else
        --usedLines;
    }
    output[sample*(short)outputL + line] = output[sample*(short)outputL + line] / usedLines;
  }
}

void mitk::BeamformingDASFilter::DASQuadraticLine(double* input, double* output, double inputDim[2], double outputDim[2], const short& line, double* apodisation, const short& apodArraySize)
{
  double& inputS = inputDim[1];
  double& inputL = inputDim[0];

  double& outputS = outputDim[1];
  double& outputL = outputDim[0];

  short AddSample = 0;
  short maxLine = 0;
  short minLine = 0;
  double delayMultiplicator = 0;
  double l_i = 0;
  double s_i = 0;

  double l = 0;
  double x = 0;
  double root = 0;

  double part = 0.07 * inputL;
  double tan_phi = std::tan(m_Conf.Angle / 360 * 2 * M_PI);
  double part_multiplicator = tan_phi * m_Conf.RecordTime / inputS * m_Conf.SpeedOfSound / m_Conf.Pitch * m_Conf.ReconstructionLines / m_Conf.TransducerElements;
  double apod_mult = 1;

  double mult = 0;
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
    minLine = (short)std::max((l_i - part), 0.0);
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

void mitk::BeamformingDASFilter::DASSphericalLine(double* input, double* output, double inputDim[2], double outputDim[2], const short& line, double* apodisation, const short& apodArraySize)
{
  double& inputS = inputDim[1];
  double& inputL = inputDim[0];

  double& outputS = outputDim[1];
  double& outputL = outputDim[0];

  short AddSample = 0;
  short maxLine = 0;
  short minLine = 0;
  double delayMultiplicator = 0;
  double l_i = 0;
  double s_i = 0;

  double l = 0;
  double x = 0;
  double root = 0;

  double part = 0.07 * inputL;
  double tan_phi = std::tan(m_Conf.Angle / 360 * 2 * M_PI);
  double part_multiplicator = tan_phi * m_Conf.RecordTime / inputS * m_Conf.SpeedOfSound / m_Conf.Pitch * m_Conf.ReconstructionLines / m_Conf.TransducerElements;
  double apod_mult = 1;

  double mult = 0;
  short usedLines = (maxLine - minLine);

  //exact delay

  l_i = (double)line / outputL * inputL;

  for (short sample = 0; sample < outputS; ++sample)
  {
    s_i = (double)sample / outputS * inputS;

    part = part_multiplicator*s_i;

    if (part < 1)
      part = 1;

    maxLine = (short)std::min((l_i + part) + 1, inputL);
    minLine = (short)std::max((l_i - part), 0.0);
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

mitk::Image::Pointer mitk::BeamformingDASFilter::BandpassFilter(mitk::Image::Pointer data)
{
  // do a fourier transform, multiply with an appropriate window for the filter, and transform back
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

itk::Image<double, 3U>::Pointer mitk::BeamformingDASFilter::BPFunction(mitk::Image::Pointer reference, int width, int center)
{
  double* imageData = new double[reference->GetDimension(0)*reference->GetDimension(1)];

  for (int sample = 0; sample < reference->GetDimension(1); ++sample)
  {
    imageData[reference->GetDimension(0)*sample] = 0;
  }

  /* // tukey window
  double alpha = m_Conf.BPFalloff;

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
  double d = center - width / 2;
  double l = center + width / 2;

  for (int n = 0; n < reference->GetDimension(1); ++n)
  {
    imageData[reference->GetDimension(0)*n] = 1 / (1 + pow(((double)center - (double)n) / ((double)width / 2), 2 * m_Conf.ButterworthOrder));
  }

  // copy and paste to all lines
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
