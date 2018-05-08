/*===================================================================
mitkCastToFloatImageFilter
The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "mitkBandpassFilter.h"

#include "../ITKFilter/ITKUltrasound/itkFFT1DComplexConjugateToRealImageFilter.h"
#include "../ITKFilter/ITKUltrasound/itkFFT1DRealToComplexConjugateImageFilter.h"
#include "mitkImageCast.h"
#include "itkMultiplyImageFilter.h"
#include "mitkITKImageImport.h"
#include "mitkIOUtil.h"

mitk::BandpassFilter::BandpassFilter() :
  m_HighPass(0),
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

itk::Image<float, 3U>::Pointer BPFunction(mitk::Image::Pointer reference,
  float cutoffFrequencyPixelHighPass,
  float cutoffFrequencyPixelLowPass,
  float alphaHighPass, float alphaLowPass)
{
  float* imageData = new float[reference->GetDimension(0)*reference->GetDimension(1)];

  float width = cutoffFrequencyPixelLowPass - cutoffFrequencyPixelHighPass;
  float center = cutoffFrequencyPixelHighPass / 2.0 + width / 2.0;

  for (unsigned int n = 0; n < reference->GetDimension(1); ++n)
  {
    imageData[reference->GetDimension(0)*n] = 0;
  }
  for (int n = 0; n < width; ++n)
  {
    imageData[reference->GetDimension(0)*n] = 1;
    if (n <= (alphaHighPass*(width - 1)) / 2.0)
    {
      if (alphaHighPass > 0.00001)
      {
        imageData[reference->GetDimension(0)*(int)(n + center - (width / 2))] =
          (1 + cos(itk::Math::pi*(2 * n / (alphaHighPass*(width - 1)) - 1))) / 2;
      }
      else
      {
        imageData[reference->GetDimension(0)*(int)(n + center - (width / 2))] = 1;
      }
    }
    else if (n >= (width - 1)*(1 - alphaLowPass / 2))
    {
      if (alphaLowPass > 0.00001)
      {
        imageData[reference->GetDimension(0)*(int)(n + center - (width / 2))] =
          (1 + cos(itk::Math::pi*(2 * n / (alphaLowPass*(width - 1)) + 1 - 2 / alphaLowPass))) / 2;
      }
      else
      {
        imageData[reference->GetDimension(0)*(int)(n + center - (width / 2))] = 1;
      }
    }
  }

  for (unsigned int n = reference->GetDimension(1) / 2; n < reference->GetDimension(1); ++n)
  {
    imageData[reference->GetDimension(0)*n] = imageData[(reference->GetDimension(1) - (n + 1)) * reference->GetDimension(0)];
  }

  for (unsigned int line = 1; line < reference->GetDimension(0); ++line)
  {
    for (unsigned int sample = 0; sample < reference->GetDimension(1); ++sample)
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

  for (unsigned int slice = 0; slice < reference->GetDimension(2); ++slice)
  {
    for (unsigned int line = 0; line < reference->GetDimension(0); ++line)
    {
      for (unsigned int sample = 0; sample < reference->GetDimension(1); ++sample)
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

void mitk::BandpassFilter::GenerateData()
{
  auto input = GetInput();

  if (!(input->GetPixelType().GetTypeAsString() == "scalar (float)" || input->GetPixelType().GetTypeAsString() == " (float)"))
  {
    MITK_ERROR << "Pixel type is not float, abort";
    mitkThrow() << "Pixel type is not float, abort";
  }

  auto output = GetOutput();
  mitk::Image::Pointer resampledInput;

  bool powerOfTwo = false;
  int finalPower = 0;
  for (int i = 1; pow(2, i) <= input->GetDimension(1); ++i)
  {
    finalPower = i;
    if (pow(2, i) == input->GetDimension(1))
    {
      powerOfTwo = true;
    }
  }
  if (!powerOfTwo)
  {
    double spacing_x = input->GetGeometry()->GetSpacing()[0];
    double spacing_y = input->GetGeometry()->GetSpacing()[1] *
      (((double)input->GetDimensions()[1]) / pow(2, finalPower + 1));
    double dim[2] = { spacing_x, spacing_y };
    resampledInput = m_FilterService->ApplyResampling(input, dim);
  }

  // do a fourier transform, multiply with an appropriate window for the filter, and transform back
  typedef itk::Image< float, 3 > RealImageType;
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
  catch (itk::ExceptionObject & error)
  {
    std::cerr << "Error: " << error << std::endl;
    MITK_ERROR << "Bandpass could not be applied";
    mitkThrow() << "bandpass error";
  }

  MITK_INFO << "HighPass: " << m_HighPass;
  MITK_INFO << "LowPass: " << m_LowPass;

  float singleVoxel = 1.0f / ((float)resampledInput->GetGeometry()->GetSpacing()[1] / 1000);
  float cutoffPixelHighPass = std::min((m_HighPass / singleVoxel) / 2, (float)resampledInput->GetDimension(1) / 2.0f);
  float cutoffPixelLowPass = std::min((m_LowPass / singleVoxel) / 2, (float)resampledInput->GetDimension(1) / 2.0f - cutoffPixelHighPass);

  MITK_INFO << "cutoffPixelHighPass: " << cutoffPixelHighPass;
  MITK_INFO << "cutoffPixelLowPass: " << cutoffPixelLowPass;

  RealImageType::Pointer fftMultiplicator = BPFunction(resampledInput,
    cutoffPixelHighPass,
    cutoffPixelLowPass,
    m_HighPassAlpha,
    m_LowPassAlpha);

  mitk::Image::Pointer testImage;
  mitk::CastToMitkImage(fftMultiplicator, testImage);

  typedef itk::MultiplyImageFilter< ComplexImageType,
    RealImageType,
    ComplexImageType >
    MultiplyFilterType;
  MultiplyFilterType::Pointer multiplyFilter = MultiplyFilterType::New();
  multiplyFilter->SetInput1(forwardFFTFilter->GetOutput());
  multiplyFilter->SetInput2(fftMultiplicator);

  typedef itk::FFT1DComplexConjugateToRealImageFilter< ComplexImageType, RealImageType > InverseFilterType;
  InverseFilterType::Pointer inverseFFTFilter = InverseFilterType::New();
  inverseFFTFilter->SetInput(multiplyFilter->GetOutput());
  inverseFFTFilter->SetDirection(1);

  GrabItkImageMemory(inverseFFTFilter->GetOutput(), output);
}
