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

mitk::Image::Pointer mitk::PhotoacousticFilterService::ApplyBmodeFilter(mitk::Image::Pointer inputImage, BModeMethod method, bool UseGPU, bool UseLogFilter, float resampleSpacing)
{
  // the image needs to be of floating point type for the envelope filter to work; the casting is done automatically by the CastToItkImage
  typedef itk::Image< float, 3 > itkFloatImageType;
  typedef itk::IdentityTransform<double, 3> TransformType;

  if (method == BModeMethod::Abs)
  {
    mitk::Image::Pointer input;
    mitk::Image::Pointer out;
    if (inputImage->GetPixelType().GetTypeAsString() == "scalar (float)" || inputImage->GetPixelType().GetTypeAsString() == " (float)")
      input = inputImage;
    else
      input = ApplyCropping(inputImage, 0, 0, 0, 0, 0, 0);

    if (!UseGPU)
    {
      PhotoacousticBModeFilter::Pointer filter = PhotoacousticBModeFilter::New();
      filter->SetParameters(UseLogFilter);
      filter->SetInput(input);
      filter->Update();

      out = filter->GetOutput();

      if (resampleSpacing == 0)
        return out;
    }
#ifdef PHOTOACOUSTICS_USE_GPU
    else
    {
      PhotoacousticOCLBModeFilter::Pointer filter = PhotoacousticOCLBModeFilter::New();
      filter->SetParameters(UseLogFilter);
      filter->SetInput(input);
      filter->Update();

      out = filter->GetOutput();

      if (resampleSpacing == 0)
        return out;
    }
#endif

    typedef itk::ResampleImageFilter < itkFloatImageType, itkFloatImageType > ResampleImageFilter;
    ResampleImageFilter::Pointer resampleImageFilter = ResampleImageFilter::New();

    itkFloatImageType::Pointer itkImage;
    mitk::CastToItkImage(out, itkImage);
    itkFloatImageType::SpacingType outputSpacing;
    itkFloatImageType::SizeType inputSize = itkImage->GetLargestPossibleRegion().GetSize();
    itkFloatImageType::SizeType outputSize = inputSize;

    outputSpacing[0] = itkImage->GetSpacing()[0];
    outputSpacing[1] = resampleSpacing;
    outputSpacing[2] = itkImage->GetSpacing()[2];

    outputSize[1] = inputSize[1] * itkImage->GetSpacing()[1] / outputSpacing[1];

    typedef itk::IdentityTransform<double, 3> TransformType;
    resampleImageFilter->SetInput(itkImage);
    resampleImageFilter->SetSize(outputSize);
    resampleImageFilter->SetOutputSpacing(outputSpacing);
    resampleImageFilter->SetTransform(TransformType::New());

    resampleImageFilter->UpdateLargestPossibleRegion();
    return mitk::GrabItkImageMemory(resampleImageFilter->GetOutput());
  }
  else if (method == BModeMethod::EnvelopeDetection)
  {
    typedef itk::BModeImageFilter < itkFloatImageType, itkFloatImageType > BModeFilterType;
    BModeFilterType::Pointer bModeFilter = BModeFilterType::New();  // LogFilter

    typedef itk::PhotoacousticBModeImageFilter < itkFloatImageType, itkFloatImageType > PhotoacousticBModeImageFilter;
    PhotoacousticBModeImageFilter::Pointer photoacousticBModeFilter = PhotoacousticBModeImageFilter::New(); // No LogFilter

    typedef itk::ResampleImageFilter < itkFloatImageType, itkFloatImageType > ResampleImageFilter;
    ResampleImageFilter::Pointer resampleImageFilter = ResampleImageFilter::New();

    itkFloatImageType::Pointer itkImage;

    mitk::CastToItkImage(inputImage, itkImage);

    itkFloatImageType::Pointer bmode;

    if (UseLogFilter)
    {
      bModeFilter->SetInput(itkImage);
      bModeFilter->SetDirection(1);
      bmode = bModeFilter->GetOutput();
    }
    else
    {
      photoacousticBModeFilter->SetInput(itkImage);
      photoacousticBModeFilter->SetDirection(1);
      bmode = photoacousticBModeFilter->GetOutput();
    }

    // resampleSpacing == 0 means: do no resampling
    if (resampleSpacing == 0)
    {
      return mitk::GrabItkImageMemory(bmode);
    }

    itkFloatImageType::SpacingType outputSpacing;
    itkFloatImageType::SizeType inputSize = itkImage->GetLargestPossibleRegion().GetSize();
    itkFloatImageType::SizeType outputSize = inputSize;

    outputSpacing[0] = itkImage->GetSpacing()[0];
    outputSpacing[1] = resampleSpacing;
    outputSpacing[2] = itkImage->GetSpacing()[2];

    outputSize[1] = inputSize[1] * itkImage->GetSpacing()[1] / outputSpacing[1];

    resampleImageFilter->SetInput(bmode);
    resampleImageFilter->SetSize(outputSize);
    resampleImageFilter->SetOutputSpacing(outputSpacing);
    resampleImageFilter->SetTransform(TransformType::New());

    resampleImageFilter->UpdateLargestPossibleRegion();
    return mitk::GrabItkImageMemory(resampleImageFilter->GetOutput());
  }
  return nullptr;
}

mitk::Image::Pointer mitk::PhotoacousticFilterService::ApplyResampling(mitk::Image::Pointer inputImage, unsigned int outputSize[2])
{
  typedef itk::Image< float, 3 > itkFloatImageType;

  typedef itk::ResampleImageFilter < itkFloatImageType, itkFloatImageType > ResampleImageFilter;
  ResampleImageFilter::Pointer resampleImageFilter = ResampleImageFilter::New();
  typedef itk::LinearInterpolateImageFunction<itkFloatImageType, double> T_Interpolator;
  itkFloatImageType::Pointer itkImage;

  mitk::CastToItkImage(inputImage, itkImage);

  itkFloatImageType::SpacingType outputSpacingItk;
  itkFloatImageType::SizeType inputSizeItk = itkImage->GetLargestPossibleRegion().GetSize();
  itkFloatImageType::SizeType outputSizeItk = inputSizeItk;

  outputSizeItk[0] = outputSize[0];
  outputSizeItk[1] = outputSize[1];
  outputSizeItk[2] = inputSizeItk[2];

  outputSpacingItk[0] = itkImage->GetSpacing()[0] * (static_cast<double>(inputSizeItk[0]) / static_cast<double>(outputSizeItk[0]));
  outputSpacingItk[1] = itkImage->GetSpacing()[1] * (static_cast<double>(inputSizeItk[1]) / static_cast<double>(outputSizeItk[1]));
  outputSpacingItk[2] = itkImage->GetSpacing()[2];

  typedef itk::IdentityTransform<double, 3> TransformType;
  T_Interpolator::Pointer _pInterpolator = T_Interpolator::New();
  resampleImageFilter->SetInput(itkImage);
  resampleImageFilter->SetSize(outputSizeItk);
  resampleImageFilter->SetOutputSpacing(outputSpacingItk);
  resampleImageFilter->SetTransform(TransformType::New());
  resampleImageFilter->SetInterpolator(_pInterpolator);

  resampleImageFilter->UpdateLargestPossibleRegion();
  return mitk::GrabItkImageMemory(resampleImageFilter->GetOutput());
}

mitk::Image::Pointer mitk::PhotoacousticFilterService::ApplyCropping(mitk::Image::Pointer inputImage,
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

mitk::Image::Pointer mitk::PhotoacousticFilterService::ApplyBeamforming(mitk::Image::Pointer inputImage,
  BeamformingSettings::Pointer config, std::function<void(int, std::string)> progressHandle)
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

mitk::Image::Pointer mitk::PhotoacousticFilterService::BandpassFilter(mitk::Image::Pointer data, float recordTime,
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
    unsigned int dim[2] = { data->GetDimension(0), (unsigned int)pow(2,finalPower + 1) };
    data = ApplyResampling(data, dim);
  }

  MITK_INFO << data->GetDimension(0);

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
    MITK_WARN << "Bandpass could not be applied";
    return data;
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

  /*itk::ComplexToModulusImageFilter<ComplexImageType, RealImageType>::Pointer toReal = itk::ComplexToModulusImageFilter<ComplexImageType, RealImageType>::New();
  toReal->SetInput(forwardFFTFilter->GetOutput());
  return GrabItkImageMemory(toReal->GetOutput());
  return GrabItkImageMemory(fftMultiplicator);   *///DEBUG

  typedef itk::FFT1DComplexConjugateToRealImageFilter< ComplexImageType, RealImageType > InverseFilterType;
  InverseFilterType::Pointer inverseFFTFilter = InverseFilterType::New();
  inverseFFTFilter->SetInput(multiplyFilter->GetOutput());
  inverseFFTFilter->SetDirection(1);

  return GrabItkImageMemory(inverseFFTFilter->GetOutput());
}

itk::Image<float, 3U>::Pointer mitk::PhotoacousticFilterService::BPFunction(mitk::Image::Pointer reference,
  int cutoffFrequencyPixelHighPass,
  int cutoffFrequencyPixelLowPass,
  float alphaHighPass, float alphaLowPass)
{
  float* imageData = new float[reference->GetDimension(0)*reference->GetDimension(1)];

  float width = reference->GetDimension(1) / 2.0 - (float)cutoffFrequencyPixelHighPass - (float)cutoffFrequencyPixelLowPass;
  float center = (float)cutoffFrequencyPixelHighPass / 2.0 + width / 2.0;

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
    else if (n >= (width - 1)*(1 - alphaLowPass / 2)) //???
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
    //MITK_INFO << "n:" << n << " is " << imageData[reference->GetDimension(0)*(int)(n + center - (width / 2))];
  }
  MITK_INFO << "width: " << width << ", center: " << center << ", alphaHighPass: " << alphaHighPass << ", alphaLowPass: " << alphaLowPass;

  // mirror the first half of the image
  for (unsigned int n = reference->GetDimension(1) / 2; n < reference->GetDimension(1); ++n)
  {
    imageData[reference->GetDimension(0)*n] = imageData[(reference->GetDimension(1) - (n + 1)) * reference->GetDimension(0)];
  }

  // copy and paste to all lines
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
