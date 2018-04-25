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

#include "mitkPhotoacousticFilterService.h"
#include "../ITKFilter/ITKUltrasound/itkBModeImageFilter.h"
#include "../ITKFilter/itkPhotoacousticBModeImageFilter.h"
#include "mitkImageCast.h"
#include "mitkITKImageImport.h"
#include "mitkBeamformingFilter.h"
#include <chrono>
#include <mitkIOUtil.h>
#include <mitkCropImageFilter.h>
#include "./OpenCLFilter/mitkPhotoacousticBModeFilter.h"
#include "mitkConvert2Dto3DImageFilter.h"
#include <mitkCastToFloatImageFilter.h>

// itk dependencies
#include "itkImage.h"
#include "itkResampleImageFilter.h"
#include "itkCastImageFilter.h"
#include "itkCropImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"
#include "itkIntensityWindowingImageFilter.h"
#include <itkIndex.h>
#include "itkMultiplyImageFilter.h"
#include "itkBSplineInterpolateImageFunction.h"
#include <mitkImageToItk.h>

// needed itk image filters
#include "mitkITKImageImport.h"
#include "itkFFTShiftImageFilter.h"
#include "itkMultiplyImageFilter.h"
#include "itkComplexToModulusImageFilter.h"
#include <itkAddImageFilter.h>
#include "../ITKFilter/ITKUltrasound/itkFFT1DComplexConjugateToRealImageFilter.h"
#include "../ITKFilter/ITKUltrasound/itkFFT1DRealToComplexConjugateImageFilter.h"

mitk::PhotoacousticFilterService::PhotoacousticFilterService()
{
  MITK_INFO << "[PhotoacousticFilterService] created filter service";
}

mitk::PhotoacousticFilterService::~PhotoacousticFilterService()
{
  MITK_INFO << "[PhotoacousticFilterService] destructed filter service";
}

mitk::Image::Pointer mitk::PhotoacousticFilterService::ApplyBmodeFilter(mitk::Image::Pointer inputImage, BModeMethod method, bool UseLogFilter)
{
  // the image needs to be of floating point type for the envelope filter to work; the casting is done automatically by the CastToItkImage
  typedef itk::Image< float, 3 > itkFloatImageType;

  if (inputImage.IsNull() ||
    !(inputImage->GetPixelType().GetTypeAsString() == "scalar (float)"
      || inputImage->GetPixelType().GetTypeAsString() == " (float)"))
  {
    MITK_ERROR << "BMode Filter can only handle float image types.";
    mitkThrow() << "BMode Filter can only handle float image types.";
  }

  if (method == BModeMethod::Abs)
  {
    PhotoacousticBModeFilter::Pointer filter = PhotoacousticBModeFilter::New();
    filter->UseLogFilter(UseLogFilter);
    filter->SetInput(inputImage);
    filter->Update();
    return filter->GetOutput();
  }

  typedef itk::BModeImageFilter < itkFloatImageType, itkFloatImageType > BModeFilterType;
  BModeFilterType::Pointer bModeFilter = BModeFilterType::New();  // LogFilter

  typedef itk::PhotoacousticBModeImageFilter < itkFloatImageType, itkFloatImageType > PhotoacousticBModeImageFilter;
  PhotoacousticBModeImageFilter::Pointer photoacousticBModeFilter = PhotoacousticBModeImageFilter::New(); // No LogFilter

  typedef itk::ResampleImageFilter < itkFloatImageType, itkFloatImageType > ResampleImageFilter;
  ResampleImageFilter::Pointer resampleImageFilter = ResampleImageFilter::New();

  itkFloatImageType::Pointer itkImage;

  mitk::CastToItkImage(inputImage, itkImage);

  if (UseLogFilter)
  {
    bModeFilter->SetInput(itkImage);
    bModeFilter->SetDirection(1);
    itkImage = bModeFilter->GetOutput();
  }
  else
  {
    photoacousticBModeFilter->SetInput(itkImage);
    photoacousticBModeFilter->SetDirection(1);
    itkImage = photoacousticBModeFilter->GetOutput();
  }

  return mitk::GrabItkImageMemory(itkImage);
}

mitk::Image::Pointer mitk::PhotoacousticFilterService::ApplyResampling(
  mitk::Image::Pointer inputImage,
  double outputSpacing[2])
{
  typedef itk::Image< float, 3 > itkFloatImageType;

  typedef itk::ResampleImageFilter < itkFloatImageType, itkFloatImageType > ResampleImageFilter;
  ResampleImageFilter::Pointer resampleImageFilter = ResampleImageFilter::New();

  itkFloatImageType::Pointer itkImage;

  mitk::CastToItkImage(inputImage, itkImage);

  itkFloatImageType::SpacingType outputSpacingItk;
  itkFloatImageType::SizeType inputSizeItk = itkImage->GetLargestPossibleRegion().GetSize();
  itkFloatImageType::SizeType outputSizeItk = inputSizeItk;

  outputSpacingItk[0] = outputSpacing[0];
  outputSpacingItk[1] = outputSpacing[1];
  outputSpacingItk[2] = itkImage->GetSpacing()[2];

  outputSizeItk[0] = outputSizeItk[0] * (inputImage->GetGeometry()->GetSpacing()[0] / outputSpacing[0]);
  outputSizeItk[1] = outputSizeItk[1] * (inputImage->GetGeometry()->GetSpacing()[1] / outputSpacing[1]);

  resampleImageFilter->SetInput(itkImage);
  resampleImageFilter->SetSize(outputSizeItk);
  resampleImageFilter->SetOutputSpacing(outputSpacingItk);

  resampleImageFilter->UpdateLargestPossibleRegion();
  return mitk::GrabItkImageMemory(resampleImageFilter->GetOutput());
}

mitk::Image::Pointer mitk::PhotoacousticFilterService::ApplyCropping(
  mitk::Image::Pointer inputImage,
  int above, int below,
  int right, int left,
  int zStart, int zEnd)
{
  mitk::CropImageFilter::Pointer cropImageFilter = mitk::CropImageFilter::New();
  cropImageFilter->SetInput(inputImage);
  cropImageFilter->SetXPixelsCropStart(left);
  cropImageFilter->SetXPixelsCropEnd(right);
  cropImageFilter->SetYPixelsCropStart(above);
  cropImageFilter->SetYPixelsCropEnd(below);
  cropImageFilter->SetZPixelsCropStart(zStart);
  cropImageFilter->SetZPixelsCropEnd(zEnd);
  cropImageFilter->Update();
  return cropImageFilter->GetOutput();
}

mitk::Image::Pointer mitk::PhotoacousticFilterService::ApplyBeamforming(
  mitk::Image::Pointer inputImage,
  BeamformingSettings::Pointer config,
  std::function<void(int, std::string)> progressHandle)
{
  Image::Pointer processedImage = mitk::Image::New();

  if (inputImage->GetDimension() != 3)
  {
    mitk::Convert2Dto3DImageFilter::Pointer dimensionImageFilter = mitk::Convert2Dto3DImageFilter::New();
    dimensionImageFilter->SetInput(inputImage);
    dimensionImageFilter->Update();
    processedImage = dimensionImageFilter->GetOutput();
  }
  else
  {
    processedImage = inputImage;
  }

  // perform the beamforming
  m_BeamformingFilter = mitk::BeamformingFilter::New(config);
  m_BeamformingFilter->SetInput(processedImage);
  m_BeamformingFilter->SetProgressHandle(progressHandle);
  m_BeamformingFilter->UpdateLargestPossibleRegion();

  processedImage = m_BeamformingFilter->GetOutput();

  return processedImage;
}

mitk::Image::Pointer mitk::PhotoacousticFilterService::BandpassFilter(
  mitk::Image::Pointer data, float recordTime,
  float BPHighPass, float BPLowPass,
  float alphaHighPass, float alphaLowPass)
{
  bool powerOfTwo = false;
  int finalPower = 0;
  for (int i = 1; pow(2, i) <= data->GetDimension(1); ++i)
  {
    finalPower = i;
    if (pow(2, i) == data->GetDimension(1))
    {
      powerOfTwo = true;
    }
  }
  if (!powerOfTwo)
  {
    double spacing_x = data->GetGeometry()->GetSpacing()[0];
    double spacing_y = data->GetGeometry()->GetSpacing()[1] * (((double)data->GetDimensions()[1]) / pow(2, finalPower + 1));
    double dim[2] = { spacing_x, spacing_y };
    data = ApplyResampling(data, dim);
  }

  MITK_INFO << data->GetDimension(0);
  MITK_INFO << data->GetDimension(1);

  // do a fourier transform, multiply with an appropriate window for the filter, and transform back
  typedef itk::Image< float, 3 > RealImageType;
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
    MITK_ERROR << "Bandpass could not be applied";
    mitkThrow() << "bandpass error";
  }

  float singleVoxel = 1 / (recordTime / data->GetDimension(1)) / 2 / 1000;
  float cutoffPixelHighPass = std::min(BPHighPass / singleVoxel, (float)data->GetDimension(1) / 2);
  float cutoffPixelLowPass = std::min(BPLowPass / singleVoxel, (float)data->GetDimension(1) / 2 - cutoffPixelHighPass);

  RealImageType::Pointer fftMultiplicator = BPFunction(data, cutoffPixelHighPass, cutoffPixelLowPass, alphaHighPass, alphaLowPass);

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

  return GrabItkImageMemory(inverseFFTFilter->GetOutput());
}

itk::Image<float, 3U>::Pointer mitk::PhotoacousticFilterService::BPFunction(mitk::Image::Pointer reference,
  float cutoffFrequencyPixelHighPass,
  float cutoffFrequencyPixelLowPass,
  float alphaHighPass, float alphaLowPass)
{
  float* imageData = new float[reference->GetDimension(0)*reference->GetDimension(1)];

  float width = reference->GetDimension(1) / 2.0 - cutoffFrequencyPixelHighPass - cutoffFrequencyPixelLowPass;
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
