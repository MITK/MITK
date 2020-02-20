/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkPAVolumeManipulator.h"
#include "mitkPAExceptions.h"
#include "mitkPAInSilicoTissueVolume.h"
#include "itkResampleImageFilter.h"
#include "itkGaussianInterpolateImageFunction.h"

// Includes for image casting between ITK and MITK
#include <mitkImageCast.h>
#include <mitkITKImageImport.h>

#include <mitkIOUtil.h>

mitk::pa::VolumeManipulator::VolumeManipulator() {}

mitk::pa::VolumeManipulator::~VolumeManipulator() {}

void mitk::pa::VolumeManipulator::ThresholdImage(Volume::Pointer image, double threshold)
{
  for (unsigned int z = 0; z < image->GetZDim(); z++)
    for (unsigned int y = 0; y < image->GetYDim(); y++)
      for (unsigned int x = 0; x < image->GetXDim(); x++)
        if (image->GetData(x, y, z) > threshold)
          image->SetData(1, x, y, z);
        else
          image->SetData(0, x, y, z);
}

void mitk::pa::VolumeManipulator::MultiplyImage(Volume::Pointer image, double factor)
{
  for (unsigned int z = 0; z < image->GetZDim(); z++)
    for (unsigned int y = 0; y < image->GetYDim(); y++)
      for (unsigned int x = 0; x < image->GetXDim(); x++)
        image->SetData(image->GetData(x, y, z)*factor, x, y, z);
}

void mitk::pa::VolumeManipulator::Log10Image(Volume::Pointer image)
{
  for (unsigned int z = 0; z < image->GetZDim(); z++)
    for (unsigned int y = 0; y < image->GetYDim(); y++)
      for (unsigned int x = 0; x < image->GetXDim(); x++)
      {
        if (image->GetData(x, y, z) < 0)
        {
          MITK_ERROR << "Signal was smaller than 0. There is an error in the input image file.";
          throw InvalidValueException("Signal was smaller than 0. There is an error in the input image file.");
        }
        image->SetData(log10(image->GetData(x, y, z)), x, y, z);
      }
}

void mitk::pa::VolumeManipulator::RescaleImage(InSilicoTissueVolume::Pointer image, double ratio)
{
  MITK_INFO << "Rescaling images..";
  double sigma = image->GetSpacing() / (ratio * 2);
  image->SetAbsorptionVolume(RescaleImage(image->GetAbsorptionVolume(), ratio, sigma));
  image->SetScatteringVolume(RescaleImage(image->GetScatteringVolume(), ratio, sigma));
  image->SetAnisotropyVolume(RescaleImage(image->GetAnisotropyVolume(), ratio, sigma));
  image->SetSegmentationVolume(RescaleImage(image->GetSegmentationVolume(), ratio, 0));
  MITK_INFO << "Rescaling images..[Done]";
}

mitk::pa::Volume::Pointer mitk::pa::VolumeManipulator::RescaleImage(Volume::Pointer image, double ratio, double sigma)
{
  MITK_INFO << "Rescaling image..";
  typedef itk::Image<double, 3> ImageType;
  typedef itk::ResampleImageFilter<ImageType, ImageType> FilterType;
  typedef itk::GaussianInterpolateImageFunction<ImageType, double> InterpolatorType;

  auto input = image->AsMitkImage();
  ImageType::Pointer itkInput = ImageType::New();
  mitk::CastToItkImage(input, itkInput);

  ImageType::SizeType outputSize;
  outputSize[0] = input->GetDimensions()[0] * ratio;
  outputSize[1] = input->GetDimensions()[1] * ratio;
  outputSize[2] = input->GetDimensions()[2] * ratio;

  FilterType::Pointer resampleImageFilter = FilterType::New();
  resampleImageFilter->SetInput(itkInput);
  resampleImageFilter->SetSize(outputSize);
  if (sigma > mitk::eps)
  {
    auto interpolator = InterpolatorType::New();
    interpolator->SetSigma(sigma);
    resampleImageFilter->SetInterpolator(interpolator);
  }
  resampleImageFilter->SetOutputSpacing(input->GetGeometry()->GetSpacing()[0] / ratio);

  MITK_INFO << "Update..";
  resampleImageFilter->UpdateLargestPossibleRegion();
  MITK_INFO << "Update..[Done]";

  ImageType::Pointer output = resampleImageFilter->GetOutput();
  mitk::Image::Pointer mitkOutput = mitk::Image::New();

  GrabItkImageMemory(output, mitkOutput);
  MITK_INFO << "Rescaling image..[Done]";
  return Volume::New(mitkOutput);
}

/**
* @brief Fast 3D Gaussian convolution IIR approximation
* @param paVolume
* @param sigma
* @author Pascal Getreuer <getreuer@gmail.com>
*
* Copyright (c) 2011, Pascal Getreuer
* All rights reserved.
*
* This program is free software: you can redistribute it and/or modify it
* under the terms of the simplified BSD license.
*
* You should have received a copy of these licenses along with this program.
* If not, see <http://www.opensource.org/licenses/bsd-license.html>.
*/
void mitk::pa::VolumeManipulator::GaussianBlur3D(mitk::pa::Volume::Pointer paVolume, double sigma)
{
  double* volume = paVolume->GetData();
  long width = paVolume->GetYDim();
  long height = paVolume->GetXDim();
  long depth = paVolume->GetZDim();
  const long plane = width*height;
  const long numel = plane*depth;
  double lambda, dnu;
  double nu, boundaryscale, postscale;
  double *ptr;
  long i, x, y, z;
  int step;

  if (sigma <= 0)
    return;

  lambda = (sigma*sigma) / (8.0);
  dnu = (1.0 + 2.0*lambda - sqrt(1.0 + 4.0*lambda)) / (2.0*lambda);
  nu = dnu;
  boundaryscale = 1.0 / (1.0 - dnu);
  postscale = pow(dnu / lambda, 12);

  /* Filter horizontally along each row */
  for (z = 0; z < depth; z++)
  {
    for (y = 0; y < height; y++)
    {
      for (step = 0; step < 4; step++)
      {
        ptr = volume + width*(y + height*z);
        ptr[0] *= boundaryscale;

        /* Filter rightwards */
        for (x = 1; x < width; x++)
        {
          ptr[x] += nu*ptr[x - 1];
        }

        ptr[x = width - 1] *= boundaryscale;
        /* Filter leftwards */
        for (; x > 0; x--)
        {
          ptr[x - 1] += nu*ptr[x];
        }
      }
    }
  }
  /* Filter vertically along each column */
  for (z = 0; z < depth; z++)
  {
    for (x = 0; x < width; x++)
    {
      for (step = 0; step < 4; step++)
      {
        ptr = volume + x + plane*z;
        ptr[0] *= boundaryscale;

        /* Filter downwards */
        for (i = width; i < plane; i += width)
        {
          ptr[i] += nu*ptr[i - width];
        }

        ptr[i = plane - width] *= boundaryscale;

        /* Filter upwards */
        for (; i > 0; i -= width)
        {
          ptr[i - width] += nu*ptr[i];
        }
      }
    }
  }

  /* Filter along z-dimension */
  for (y = 0; y < height; y++)
  {
    for (x = 0; x < width; x++)
    {
      for (step = 0; step < 4; step++)
      {
        ptr = volume + x + width*y;
        ptr[0] *= boundaryscale;

        for (i = plane; i < numel; i += plane)
        {
          ptr[i] += nu*ptr[i - plane];
        }

        ptr[i = numel - plane] *= boundaryscale;

        for (; i > 0; i -= plane)
        {
          ptr[i - plane] += nu*ptr[i];
        }
      }
    }
  }

  for (i = 0; i < numel; i++)
  {
    volume[i] *= postscale;
  }
}
