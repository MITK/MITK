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
#include "Algorithms/ITKUltrasound/itkBModeImageFilter.h"
#include "Algorithms/itkPhotoacousticBModeImageFilter.h"
#include "mitkImageCast.h"
#include "mitkITKImageImport.h"
#include "Algorithms/mitkPhotoacousticBeamformingDASFilter.h"
#include "Algorithms/mitkPhotoacousticBeamformingDMASFilter.h"
#include <chrono>

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
  typedef itk::BSplineInterpolateImageFunction<itkFloatImageType, double, double> T_Interpolator;
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
  _pInterpolator->SetSplineOrder(3);
  resampleImageFilter->SetInterpolator(_pInterpolator);

  resampleImageFilter->UpdateLargestPossibleRegion();
  return mitk::GrabItkImageMemory(resampleImageFilter->GetOutput());
}

mitk::Image::Pointer mitk::PhotoacousticImage::ApplyBeamformingDAS(mitk::Image::Pointer inputImage, BeamformingDASFilter::beamformingSettings config)
{
  BeamformingDASFilter::Pointer Beamformer = BeamformingDASFilter::New();

  Image::Pointer resizedImage = inputImage;

  if (inputImage->GetDimension(0) != config.ReconstructionLines)
  {
    auto begin = std::chrono::high_resolution_clock::now();
    unsigned int dim[3] = { config.ReconstructionLines, inputImage->GetDimension(1), inputImage->GetDimension(2) };
    resizedImage = ApplyResampling(inputImage, dim);
    auto end = std::chrono::high_resolution_clock::now();
    MITK_INFO << "Upsampling from " << inputImage->GetDimension(0) << " to " << config.ReconstructionLines << " lines completed in " << ((double)std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count()) / 1000000 << "ms" << std::endl;
  }

  Beamformer->SetInput(resizedImage);
  Beamformer->Configure(config);

  Beamformer->UpdateLargestPossibleRegion();
  return Beamformer->GetOutput();
}

mitk::Image::Pointer mitk::PhotoacousticImage::ApplyBeamformingDMAS(mitk::Image::Pointer inputImage, BeamformingDMASFilter::beamformingSettings config)
{
  BeamformingDMASFilter::Pointer Beamformer = BeamformingDMASFilter::New();

  Image::Pointer resizedImage = inputImage;

  if(inputImage->GetDimension(0) != config.ReconstructionLines)
  {
    auto begin = std::chrono::high_resolution_clock::now();
    unsigned int dim[3] = { config.ReconstructionLines, inputImage->GetDimension(1), inputImage->GetDimension(2) };
    resizedImage = ApplyResampling(inputImage, dim);
    auto end = std::chrono::high_resolution_clock::now();
    MITK_INFO << "Upsampling from " << inputImage->GetDimension(0) << " to " << config.ReconstructionLines << " lines completed in " << ((double)std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count()) / 1000000 << "ms" << std::endl;
  }

  Beamformer->SetInput(resizedImage);
  Beamformer->Configure(config);

  Beamformer->UpdateLargestPossibleRegion();
  return Beamformer->GetOutput();
}