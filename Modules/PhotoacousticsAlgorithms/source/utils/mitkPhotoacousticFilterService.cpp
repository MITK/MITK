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

#include "mitkITKImageImport.h"
#include <chrono>
#include <mitkCropImageFilter.h>
#include "./OpenCLFilter/mitkPhotoacousticBModeFilter.h"
#include "mitkConvert2Dto3DImageFilter.h"
#include <mitkCastToFloatImageFilter.h>
#include "../ITKFilter/ITKUltrasound/itkBModeImageFilter.h"
#include "../ITKFilter/itkPhotoacousticBModeImageFilter.h"
#include <mitkBandpassFilter.h>

// itk dependencies
#include "itkImage.h"
#include "itkResampleImageFilter.h"
#include "itkCastImageFilter.h"
#include "itkCropImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"
#include "itkIntensityWindowingImageFilter.h"
#include <itkIndex.h>
#include "itkBSplineInterpolateImageFunction.h"
#include <mitkImageToItk.h>

// needed itk image filters
#include "mitkImageCast.h"

mitk::PhotoacousticFilterService::PhotoacousticFilterService()
{
  MITK_INFO << "[PhotoacousticFilterService] created filter service";
}

mitk::PhotoacousticFilterService::~PhotoacousticFilterService()
{
  MITK_INFO << "[PhotoacousticFilterService] destructed filter service";
}

mitk::Image::Pointer mitk::PhotoacousticFilterService::ApplyBmodeFilter(
  mitk::Image::Pointer inputImage,
  BModeMethod method, bool UseLogFilter)
{
  // the image needs to be of floating point type for the envelope filter to work; the casting is done automatically by the CastToItkImage
  typedef itk::Image< float, 3 > itkFloatImageType;

  auto floatImage = ConvertToFloat(inputImage);

  if (method == BModeMethod::Abs)
  {
    PhotoacousticBModeFilter::Pointer filter = PhotoacousticBModeFilter::New();
    filter->UseLogFilter(UseLogFilter);
    filter->SetInput(floatImage);
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

  mitk::CastToItkImage(floatImage, itkImage);

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
  double *outputSpacing)
{
  typedef itk::Image< float, 3 > itkFloatImageType;

  auto floatImage = ConvertToFloat(inputImage);

  typedef itk::ResampleImageFilter < itkFloatImageType, itkFloatImageType > ResampleImageFilter;
  ResampleImageFilter::Pointer resampleImageFilter = ResampleImageFilter::New();

  itkFloatImageType::Pointer itkImage;

  mitk::CastToItkImage(floatImage, itkImage);

  itkFloatImageType::SpacingType outputSpacingItk;
  itkFloatImageType::SizeType inputSizeItk = itkImage->GetLargestPossibleRegion().GetSize();
  itkFloatImageType::SizeType outputSizeItk = inputSizeItk;

  outputSpacingItk[0] = outputSpacing[0];
  outputSpacingItk[1] = outputSpacing[1];
  outputSpacingItk[2] = itkImage->GetSpacing()[2];

  outputSizeItk[0] = outputSizeItk[0] * (floatImage->GetGeometry()->GetSpacing()[0] / outputSpacing[0]);
  outputSizeItk[1] = outputSizeItk[1] * (floatImage->GetGeometry()->GetSpacing()[1] / outputSpacing[1]);

  resampleImageFilter->SetInput(itkImage);
  resampleImageFilter->SetSize(outputSizeItk);
  resampleImageFilter->SetOutputSpacing(outputSpacingItk);

  resampleImageFilter->UpdateLargestPossibleRegion();
  return mitk::GrabItkImageMemory(resampleImageFilter->GetOutput());
}


mitk::Image::Pointer mitk::PhotoacousticFilterService::ApplyResamplingToDim(
  mitk::Image::Pointer inputImage,
  double *outputDimension)
{
  typedef itk::Image< float, 3 > itkFloatImageType;

  auto floatImage = ConvertToFloat(inputImage);

  typedef itk::ResampleImageFilter < itkFloatImageType, itkFloatImageType > ResampleImageFilter;
  ResampleImageFilter::Pointer resampleImageFilter = ResampleImageFilter::New();

  itkFloatImageType::Pointer itkImage;

  mitk::CastToItkImage(floatImage, itkImage);

  itkFloatImageType::SpacingType outputSpacingItk;
  itkFloatImageType::SizeType inputSizeItk = itkImage->GetLargestPossibleRegion().GetSize();
  itkFloatImageType::SizeType outputSizeItk = inputSizeItk;

  outputSizeItk[0] = outputDimension[0];
  outputSizeItk[1] = outputDimension[1];

  MITK_INFO << outputSizeItk[0] << " " << outputSizeItk[1];

  outputSpacingItk[0] = (double)inputSizeItk[0] / (double)outputSizeItk[0]  * floatImage->GetGeometry()->GetSpacing()[0];
  outputSpacingItk[1] = (double)inputSizeItk[1] / (double)outputSizeItk[1]  * floatImage->GetGeometry()->GetSpacing()[1];
  outputSpacingItk[2] = itkImage->GetSpacing()[2];

  MITK_INFO << outputSpacingItk[0] << " " << outputSpacingItk[1];

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
  int zStart, int zEnd,
  int* errCode)
{
  *errCode = 0;
  try
  {
    auto floatImage = ConvertToFloat(inputImage);
    mitk::CropImageFilter::Pointer cropImageFilter = mitk::CropImageFilter::New();
    cropImageFilter->SetInput(floatImage);
    cropImageFilter->SetXPixelsCropStart(left);
    cropImageFilter->SetXPixelsCropEnd(right);
    cropImageFilter->SetYPixelsCropStart(above);
    cropImageFilter->SetYPixelsCropEnd(below);
    cropImageFilter->SetZPixelsCropStart(zStart);
    cropImageFilter->SetZPixelsCropEnd(zEnd);
    cropImageFilter->Update();
    return cropImageFilter->GetOutput();
  }
  catch (mitk::Exception &e)
  {
    std::string errorMessage = "Caught unexpected exception ";
    errorMessage.append(e.what());
    MITK_ERROR << errorMessage;
    *errCode = -1;
    mitk::Image::Pointer ret = mitk::Image::New();
    unsigned int dim[3] = { 1,1,1 };
    ret->Initialize(MakeScalarPixelType<float>(), 3, dim);

    return ret;
  }
}

mitk::Image::Pointer mitk::PhotoacousticFilterService::ExtendImage(mitk::Image::Pointer inputImage, float pixelColor, unsigned int outputDimensionY)
{
  mitk::Image::Pointer outputImage = mitk::Image::New();
  unsigned int dim[] = {inputImage->GetDimension(0), outputDimensionY, inputImage->GetDimension(2)};
  outputImage->Initialize(inputImage->GetPixelType(), 3, dim);

  float *sliceData = new float[dim[0] * dim[1]];

  for (size_t i = inputImage->GetDimension(1) * dim[0]; i < dim[0] * dim[1]; ++i)
  {
    sliceData[i] = pixelColor;
  }

  for (unsigned int slice = 0; slice < dim[2]; ++slice)
  {
    mitk::ImageReadAccessor cpy(inputImage, inputImage->GetSliceData(slice));
    cpy.GetData();
    std::memcpy((void*)sliceData, cpy.GetData(), sizeof(float) * inputImage->GetDimension(1) * dim[0]);
    outputImage->SetSlice(sliceData, slice);
  }

  delete[] sliceData;
  return outputImage;
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

  m_BeamformingFilter = mitk::BeamformingFilter::New(config);
  m_BeamformingFilter->SetInput(ConvertToFloat(processedImage));
  m_BeamformingFilter->SetProgressHandle(progressHandle);
  m_BeamformingFilter->UpdateLargestPossibleRegion();

  processedImage = m_BeamformingFilter->GetOutput();

  return processedImage;
}

mitk::Image::Pointer mitk::PhotoacousticFilterService::ApplyBandpassFilter(
  mitk::Image::Pointer data,
  float BPHighPass, float BPLowPass,
  float alphaHighPass, float alphaLowPass,
  float TimeSpacing, float SpeedOfSound, bool IsBFImage)
{
  try 
  {
    auto floatData = ConvertToFloat(data);
    mitk::BandpassFilter::Pointer bandpassFilter = mitk::BandpassFilter::New();
    bandpassFilter->SetInput(floatData);
    bandpassFilter->SetHighPass(BPHighPass);
    bandpassFilter->SetLowPass(BPLowPass);
    bandpassFilter->SetHighPassAlpha(alphaHighPass);
    bandpassFilter->SetLowPassAlpha(alphaLowPass);
    bandpassFilter->SetSpeedOfSound(SpeedOfSound);
    bandpassFilter->SetTimeSpacing(TimeSpacing);
    bandpassFilter->SetIsBFImage(IsBFImage);
    bandpassFilter->Update();
    return bandpassFilter->GetOutput();
  }

  catch (mitk::Exception &e)
  {
    std::string errorMessage = "Caught unexpected exception ";
    errorMessage.append(e.what());
    MITK_ERROR << errorMessage;

    return data;
  }
}

mitk::Image::Pointer mitk::PhotoacousticFilterService::ConvertToFloat(mitk::Image::Pointer inputImage)
{
  if ((inputImage->GetPixelType().GetTypeAsString() == "scalar (float)" ||
    inputImage->GetPixelType().GetTypeAsString() == " (float)"))
  {
    return inputImage;
  }

  mitk::CastToFloatImageFilter::Pointer castToFloatImageFilter = mitk::CastToFloatImageFilter::New();
  castToFloatImageFilter->SetInput(inputImage);
  castToFloatImageFilter->Update();
  return castToFloatImageFilter->GetOutput();
}
