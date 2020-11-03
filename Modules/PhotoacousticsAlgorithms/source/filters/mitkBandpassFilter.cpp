/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#define _USE_MATH_DEFINES
#include <cmath>

#include "mitkBandpassFilter.h"

#include "../ITKFilter/ITKUltrasound/itkFFT1DComplexConjugateToRealImageFilter.h"
#include "../ITKFilter/ITKUltrasound/itkFFT1DRealToComplexConjugateImageFilter.h"
#include "itkComplexToModulusImageFilter.h"
#include "itkMultiplyImageFilter.h"
#include "mitkIOUtil.h"
#include "mitkITKImageImport.h"
#include "mitkImageCast.h"

mitk::BandpassFilter::BandpassFilter()
  : m_HighPass(0),
    m_LowPass(50),
    m_HighPassAlpha(1),
    m_LowPassAlpha(1),
    m_FilterService(mitk::PhotoacousticFilterService::New())
{
  MITK_INFO << "Instantiating BandpassFilter...";
  SetNumberOfIndexedInputs(1);
  SetNumberOfIndexedOutputs(1);
  MITK_INFO << "Instantiating BandpassFilter...[Done]";
}

mitk::BandpassFilter::~BandpassFilter()
{
  MITK_INFO << "Destructed BandpassFilter.";
}

void mitk::BandpassFilter::SanityCheckPreconditions()
{
  auto input = GetInput();

  std::string type = input->GetPixelType().GetTypeAsString();
  if (!(type == "scalar (float)" || type == " (float)"))
  {
    MITK_ERROR << "This filter can currently only handle float type images.";
    mitkThrow() << "This filter can currently only handle float type images.";
  }
  if (m_HighPass > m_LowPass)
  {
    MITK_ERROR << "High Pass is higher than Low Pass; aborting.";
    mitkThrow() << "High Pass is higher than Low Pass; aborting.";
  }
}

itk::Image<float, 3U>::Pointer BPFunction(mitk::Image::Pointer reference,
                                          float cutoffFrequencyPixelHighPass,
                                          float cutoffFrequencyPixelLowPass,
                                          float alphaHighPass,
                                          float alphaLowPass)
{
  float *imageData = new float[reference->GetDimension(0) * reference->GetDimension(1)];

  float width = cutoffFrequencyPixelLowPass - cutoffFrequencyPixelHighPass;
  float center = cutoffFrequencyPixelHighPass + width / 2.f;

  for (unsigned int n = 0; n < reference->GetDimension(1); ++n)
  {
    imageData[reference->GetDimension(0) * n] = 0;
  }
  for (int n = 0; n < width; ++n)
  {
    imageData[reference->GetDimension(0) * (int)(n + center - (width / 2.f))] = 1;
    if (n <= (alphaHighPass * (width - 1)) / 2.f)
    {
      if (alphaHighPass > 0.00001f)
      {
        imageData[reference->GetDimension(0) * (int)(n + center - (width / 2.f))] =
          (1 + cos(itk::Math::pi * (2 * n / (alphaHighPass * (width - 1)) - 1))) / 2;
      }
      else
      {
        imageData[reference->GetDimension(0) * (int)(n + center - (width / 2.f))] = 1;
      }
    }
    else if (n >= (width - 1) * (1 - alphaLowPass / 2.f))
    {
      if (alphaLowPass > 0.00001f)
      {
        imageData[reference->GetDimension(0) * (int)(n + center - (width / 2.f))] =
          (1 + cos(itk::Math::pi * (2 * n / (alphaLowPass * (width - 1)) + 1 - 2 / alphaLowPass))) / 2;
      }
      else
      {
        imageData[reference->GetDimension(0) * (int)(n + center - (width / 2.f))] = 1;
      }
    }
  }

  for (unsigned int n = reference->GetDimension(1) / 2; n < reference->GetDimension(1); ++n)
  {
    imageData[reference->GetDimension(0) * n] =
      imageData[(reference->GetDimension(1) - (n + 1)) * reference->GetDimension(0)];
  }

  for (unsigned int line = 1; line < reference->GetDimension(0); ++line)
  {
    for (unsigned int sample = 0; sample < reference->GetDimension(1); ++sample)
    {
      imageData[reference->GetDimension(0) * sample + line] = imageData[reference->GetDimension(0) * sample];
    }
  }

  typedef itk::Image<float, 3U> ImageType;
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

  for (unsigned int slice = 0; slice < reference->GetDimension(2); ++slice)
  {
    for (unsigned int line = 0; line < reference->GetDimension(0); ++line)
    {
      for (unsigned int sample = 0; sample < reference->GetDimension(1); ++sample)
      {
        pixelIndex[0] = line;
        pixelIndex[1] = sample;
        pixelIndex[2] = slice;

        image->SetPixel(pixelIndex, imageData[line + sample * reference->GetDimension(0)]);
      }
    }
  }

  delete[] imageData;

  return image;
}

void mitk::BandpassFilter::GenerateData()
{
  SanityCheckPreconditions();
  auto input = GetInput();
  auto output = GetOutput();
  mitk::Image::Pointer resampledInput = input;

  double powerOfTwo = std::log2(input->GetDimension(1));
  int finalSize = 0;
  double spacingResize = 1;

  // check if this is a power of two by checking that log2 is int
  if (std::fmod(powerOfTwo, 1.0) >= std::numeric_limits<double>::epsilon())
  {
    finalSize = (int)pow(2, std::ceil(powerOfTwo));
    double dim[2] = {(double)input->GetDimension(0), (double)finalSize };
    resampledInput = m_FilterService->ApplyResamplingToDim(input, dim);
    spacingResize = (double)input->GetDimension(1) / finalSize;
  }

  // do a fourier transform, multiply with an appropriate window for the filter, and transform back
  typedef itk::Image<float, 3> RealImageType;
  RealImageType::Pointer image;
  mitk::CastToItkImage(resampledInput, image);
  typedef itk::FFT1DRealToComplexConjugateImageFilter<RealImageType> ForwardFFTFilterType;
  typedef ForwardFFTFilterType::OutputImageType ComplexImageType;
  ForwardFFTFilterType::Pointer forwardFFTFilter = ForwardFFTFilterType::New();
  forwardFFTFilter->SetInput(image);
  forwardFFTFilter->SetDirection(1);

  try
  {
    forwardFFTFilter->UpdateOutputInformation();
  }
  catch (itk::ExceptionObject &error)
  {
    std::cerr << "Error: " << error << std::endl;
    MITK_ERROR << "Bandpass could not be applied";
    mitkThrow() << "bandpass error";
  }

  if (m_HighPass > m_LowPass)
    mitkThrow() << "High pass frequency higher than low pass frequency, abort";

  float singleVoxel = spacingResize / (m_TimeSpacing * resampledInput->GetDimension(1)); // [Hz]
  if(m_IsBFImage)
    singleVoxel = spacingResize / (resampledInput->GetGeometry()->GetSpacing()[1] / 1e3 / m_SpeedOfSound * resampledInput->GetDimension(1)); // [Hz]
  float cutoffPixelHighPass = std::min((m_HighPass / singleVoxel), (float)resampledInput->GetDimension(1) / 2.0f);
  float cutoffPixelLowPass = std::min((m_LowPass / singleVoxel), (float)resampledInput->GetDimension(1) / 2.0f);

  MITK_INFO << "SingleVoxel: " << singleVoxel;
  MITK_INFO << "HighPass: " << m_HighPass;
  MITK_INFO << "LowPass: " << m_LowPass;

  MITK_INFO << "cutoffPixelHighPass: " << cutoffPixelHighPass;
  MITK_INFO << "cutoffPixelLowPass: " << cutoffPixelLowPass;

  RealImageType::Pointer fftMultiplicator =
    BPFunction(resampledInput, cutoffPixelHighPass, cutoffPixelLowPass, m_HighPassAlpha, m_LowPassAlpha);

  typedef itk::MultiplyImageFilter<ComplexImageType, RealImageType, ComplexImageType> MultiplyFilterType;
  MultiplyFilterType::Pointer multiplyFilter = MultiplyFilterType::New();
  multiplyFilter->SetInput1(forwardFFTFilter->GetOutput());
  multiplyFilter->SetInput2(fftMultiplicator);

  /*
  GrabItkImageMemory(fftMultiplicator, output);
  mitk::IOUtil::Save(output, "D:/fftImage.nrrd");

  typedef itk::ComplexToModulusImageFilter< ComplexImageType, RealImageType > modulusType;
  modulusType::Pointer modul = modulusType::New();

  modul->SetInput(multiplyFilter->GetOutput());
  GrabItkImageMemory(modul->GetOutput(), output);
  mitk::IOUtil::Save(output, "D:/fftout.nrrd");

  modul->SetInput(forwardFFTFilter->GetOutput());
  GrabItkImageMemory(modul->GetOutput(), output);
  mitk::IOUtil::Save(output, "D:/fftin.nrrd");*/

  typedef itk::FFT1DComplexConjugateToRealImageFilter<ComplexImageType, RealImageType> InverseFilterType;
  InverseFilterType::Pointer inverseFFTFilter = InverseFilterType::New();
  inverseFFTFilter->SetInput(multiplyFilter->GetOutput());
  inverseFFTFilter->SetDirection(1);

  GrabItkImageMemory(inverseFFTFilter->GetOutput(), output);

  double dim[2] = { (double)input->GetDimension(0), (double)input->GetDimension(1) };
  auto resampledOutput = m_FilterService->ApplyResamplingToDim(output, dim);

  output->Initialize(mitk::MakeScalarPixelType<float>(), 3, input->GetDimensions());
  output->SetSpacing(resampledOutput->GetGeometry()->GetSpacing());
  ImageReadAccessor copy(resampledOutput);
  output->SetImportVolume(copy.GetData());
}
