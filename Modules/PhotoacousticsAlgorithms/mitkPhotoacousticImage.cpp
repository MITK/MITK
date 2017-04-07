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

#include "mitkPhotoacousticImage.h"
#include "itkBModeImageFilter.h"
#include "itkPhotoacousticBModeImageFilter.h"
#include "mitkImageCast.h"
#include "mitkITKImageImport.h"
#include "mitkPhotoacousticBeamformingDASFilter.h"
#include "mitkPhotoacousticBeamformingDMASFilter.h"
#include <chrono>
#include <mitkAutoCropImageFilter.h>


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

mitk::PhotoacousticImage::PhotoacousticImage()
{
  MITK_INFO << "[PhotoacousticImage Debug] created that image";
}

mitk::PhotoacousticImage::~PhotoacousticImage()
{
  MITK_INFO << "[PhotoacousticImage Debug] destroyed that image";
}

mitk::Image::Pointer mitk::PhotoacousticImage::ApplyBmodeFilter(mitk::Image::Pointer inputImage, bool UseLogFilter, float resampleSpacing)
{
  // we use this seperate ApplyBmodeFilter Method for processing of two-dimensional images

  // the image needs to be of floating point type for the envelope filter to work; the casting is done automatically by the CastToItkImage
  typedef itk::Image< float, 3 > itkFloatImageType;

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
  outputSize[0] = 256;

  outputSpacing[0] = itkImage->GetSpacing()[0] * (static_cast<double>(inputSize[0]) / static_cast<double>(outputSize[0]));
  outputSpacing[1] = resampleSpacing;
  outputSpacing[2] = 0.6;

  outputSize[1] = inputSize[1] * itkImage->GetSpacing()[1] / outputSpacing[1];

  typedef itk::IdentityTransform<double,3> TransformType;
  resampleImageFilter->SetInput(bmode);
  resampleImageFilter->SetSize(outputSize);
  resampleImageFilter->SetOutputSpacing(outputSpacing);
  resampleImageFilter->SetTransform(TransformType::New());

  resampleImageFilter->UpdateLargestPossibleRegion();
  return mitk::GrabItkImageMemory(resampleImageFilter->GetOutput());
}

//mitk::Image::Pointer mitk::PhotoacousticImage::ApplyScatteringCompensation(mitk::Image::Pointer inputImage, int scattering)
//{
//  typedef itk::Image< float, 3 > itkFloatImageType;
//  typedef itk::MultiplyImageFilter <itkFloatImageType, itkFloatImageType > MultiplyImageFilterType;

//  itkFloatImageType::Pointer itkImage;
//  mitk::CastToItkImage(inputImage, itkImage);

//  MultiplyImageFilterType::Pointer multiplyFilter = MultiplyImageFilterType::New();
//  multiplyFilter->SetInput1(itkImage);
//  multiplyFilter->SetInput2(m_FluenceCompResizedItk.at(m_ScatteringCoefficient));

//  return mitk::GrabItkImageMemory(multiplyFilter->GetOutput());
//}

mitk::Image::Pointer mitk::PhotoacousticImage::ApplyResampling(mitk::Image::Pointer inputImage, unsigned int outputSize[3])
{
  typedef itk::Image< double, 3 > itkFloatImageType;

  typedef itk::ResampleImageFilter < itkFloatImageType, itkFloatImageType > ResampleImageFilter;
  ResampleImageFilter::Pointer resampleImageFilter = ResampleImageFilter::New();
  typedef itk::LinearInterpolateImageFunction<itkFloatImageType, double> T_Interpolator;
  itkFloatImageType::Pointer itkImage;

  mitk::CastToItkImage(inputImage, itkImage);

  itkFloatImageType::SpacingType inputSpacing = itkImage->GetSpacing();
  itkFloatImageType::SpacingType outputSpacingItk;
  itkFloatImageType::SizeType inputSizeItk = itkImage->GetLargestPossibleRegion().GetSize();
  itkFloatImageType::SizeType outputSizeItk = inputSizeItk;

  outputSizeItk[0] = outputSize[0];
  outputSizeItk[1] = outputSize[1];
  outputSizeItk[2] = outputSize[2];

  outputSpacingItk[0] = itkImage->GetSpacing()[0] * (static_cast<double>(inputSizeItk[0]) / static_cast<double>(outputSizeItk[0]));
  outputSpacingItk[1] = itkImage->GetSpacing()[1] * (static_cast<double>(inputSizeItk[1]) / static_cast<double>(outputSizeItk[1]));
  outputSpacingItk[2] = itkImage->GetSpacing()[2] * (static_cast<double>(inputSizeItk[2]) / static_cast<double>(outputSizeItk[2]));

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

mitk::Image::Pointer mitk::PhotoacousticImage::ApplyCropping(mitk::Image::Pointer inputImage, int above, int below, int right, int left)
{
  double* inputData = new double[inputImage->GetDimension(0)*inputImage->GetDimension(1)*inputImage->GetDimension(2)];
  double* outputData = new double[(inputImage->GetDimension(0) - left - right)*(inputImage->GetDimension(1) - above - below)*inputImage->GetDimension(2)];

  unsigned int inputDim[3] = { inputImage->GetDimension(0), inputImage->GetDimension(1), inputImage->GetDimension(2) };
  unsigned int outputDim[3] = { inputImage->GetDimension(0) - left - right, inputImage->GetDimension(1) - above - below, inputImage->GetDimension(2) };

  ImageReadAccessor acc(inputImage);

  // as of now only those double, short, float are used at all... though it's easy to add other ones
  if (inputImage->GetPixelType().GetTypeAsString() == "scalar (double)" || inputImage->GetPixelType().GetTypeAsString() == " (double)")
  {
    inputData = (double*)acc.GetData();
  }
  else if (inputImage->GetPixelType().GetTypeAsString() == "scalar (short)" || inputImage->GetPixelType().GetTypeAsString() == " (short)")
  {
    short* inputPuffer = (short*)acc.GetData();
    for (int sl = 0; sl < inputDim[2]; ++sl)
    {
      for (int l = 0; l < inputDim[0]; ++l)
      {
        for (int s = 0; s < inputDim[1]; ++s)
        {
          inputData[l*inputDim[1] + s + sl*inputDim[0]*inputDim[1]] = (double)inputPuffer[l*inputDim[1] + s + sl*inputDim[0]*inputDim[1]];
        }
      }
    }
  }
  else if (inputImage->GetPixelType().GetTypeAsString() == "scalar (float)" || inputImage->GetPixelType().GetTypeAsString() == " (float)")
  {
    float* inputPuffer = (float*)acc.GetData();
    for (int sl = 0; sl < inputDim[2]; ++sl)
    {
      for (int l = 0; l < inputDim[0]; ++l)
      {
        for (int s = 0; s < inputDim[1]; ++s)
        {
          inputData[l*inputDim[1] + s + sl*inputDim[0]*inputDim[1]] = (double)inputPuffer[l*inputDim[1] + s + sl*inputDim[0]*inputDim[1]];
        }
      }
    }
  }
  else
  {
    MITK_INFO << "Could not determine pixel type";
  }

  for (int sl = 0; sl < outputDim[2]; ++sl)
  {
    for (int l = 0; l < outputDim[0]; ++l)
    {
      for (int s = 0; s < outputDim[1]; ++s)
      {
        outputData[l + s*(unsigned short)outputDim[0] +sl*outputDim[0]*outputDim[1]] = inputData[(l + left) + (s + above)*(unsigned short)inputDim[0] + sl*inputDim[0]*inputDim[1]];
      }
    }
  }
  mitk::Image::Pointer output = mitk::Image::New();
  output->Initialize(mitk::MakeScalarPixelType<double>(), 3, outputDim);
  output->SetSpacing(inputImage->GetGeometry()->GetSpacing());
  for (int slice = 0; slice < outputDim[2]; ++slice)
  {
    output->SetSlice(&outputData[slice*outputDim[0] * outputDim[1]], slice);
  }

  return output;
}

mitk::Image::Pointer mitk::PhotoacousticImage::ApplyBeamformingDAS(mitk::Image::Pointer inputImage, BeamformingDASFilter::beamformingSettings config, int cutoff, std::function<void(int, std::string)> progressHandle)
{
  config.RecordTime = config.RecordTime - cutoff / inputImage->GetDimension(1) * config.RecordTime; // adjust the recorded time lost by cropping
  progressHandle(0, "cropping image");
  Image::Pointer croppedImage = ApplyCropping(inputImage, cutoff, 0, 0, 0);
  Image::Pointer resizedImage = croppedImage;

  if (inputImage->GetDimension(0) != config.ReconstructionLines)
  {
    progressHandle(0, "resampling image");
    auto begin = std::chrono::high_resolution_clock::now();
    unsigned int dim[3] = { config.ReconstructionLines, croppedImage->GetDimension(1), croppedImage->GetDimension(2) };
    resizedImage = ApplyResampling(croppedImage, dim);
    auto end = std::chrono::high_resolution_clock::now();
    MITK_INFO << "Upsampling from " << inputImage->GetDimension(0) << " to " << config.ReconstructionLines << " lines completed in " << ((double)std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count()) / 1000000 << "ms" << std::endl;
  }

  BeamformingDASFilter::Pointer Beamformer = BeamformingDASFilter::New();

  Beamformer->SetInput(resizedImage);
  Beamformer->Configure(config);
  Beamformer->SetProgressHandle(progressHandle);

  Beamformer->UpdateLargestPossibleRegion();
  return Beamformer->GetOutput();
}

mitk::Image::Pointer mitk::PhotoacousticImage::ApplyBeamformingDMAS(mitk::Image::Pointer inputImage, BeamformingDMASFilter::beamformingSettings config, int cutoff, std::function<void(int, std::string)> progressHandle)
{
  config.RecordTime = config.RecordTime - cutoff / inputImage->GetDimension(1) * config.RecordTime; // adjust the recorded time lost by cropping
  progressHandle(0, "cropping image");
  Image::Pointer croppedImage = ApplyCropping(inputImage, cutoff, 0, 0, 0);
  Image::Pointer resizedImage = croppedImage;

  if(inputImage->GetDimension(0) != config.ReconstructionLines)
  {
    progressHandle(0, "resampling image");
    auto begin = std::chrono::high_resolution_clock::now();
    unsigned int dim[3] = { config.ReconstructionLines, croppedImage->GetDimension(1), croppedImage->GetDimension(2) };
    resizedImage = ApplyResampling(croppedImage, dim);
    auto end = std::chrono::high_resolution_clock::now();
    MITK_INFO << "Upsampling from " << inputImage->GetDimension(0) << " to " << config.ReconstructionLines << " lines completed in " << ((double)std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count()) / 1000000 << "ms" << std::endl;
  }

  BeamformingDMASFilter::Pointer Beamformer = BeamformingDMASFilter::New();

  Beamformer->SetInput(resizedImage);
  Beamformer->Configure(config);
  Beamformer->SetProgressHandle(progressHandle);

  Beamformer->UpdateLargestPossibleRegion();
  return Beamformer->GetOutput();
}
