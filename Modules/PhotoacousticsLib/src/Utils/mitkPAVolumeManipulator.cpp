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

#include "mitkPAVolumeManipulator.h"
#include "mitkPAExceptions.h"

mitk::pa::VolumeManipulator::VolumeManipulator() {}

mitk::pa::VolumeManipulator::~VolumeManipulator() {}

void mitk::pa::VolumeManipulator::ThresholdImage(mitk::pa::Volume::Pointer image, double threshold)
{
  for (unsigned int z = 0; z < image->GetZDim(); z++)
    for (unsigned int y = 0; y < image->GetYDim(); y++)
      for (unsigned int x = 0; x < image->GetXDim(); x++)
        if (image->GetData(x, y, z) > threshold)
          image->SetData(1, x, y, z);
        else
          image->SetData(0, x, y, z);
}

void mitk::pa::VolumeManipulator::MultiplyImage(mitk::pa::Volume::Pointer image, double factor)
{
  for (unsigned int z = 0; z < image->GetZDim(); z++)
    for (unsigned int y = 0; y < image->GetYDim(); y++)
      for (unsigned int x = 0; x < image->GetXDim(); x++)
        image->SetData(image->GetData(x, y, z)*factor, x, y, z);
}

void mitk::pa::VolumeManipulator::Log10Image(mitk::pa::Volume::Pointer image)
{
  for (unsigned int z = 0; z < image->GetZDim(); z++)
    for (unsigned int y = 0; y < image->GetYDim(); y++)
      for (unsigned int x = 0; x < image->GetXDim(); x++)
      {
        if (image->GetData(x, y, z) < 0)
        {
          MITK_ERROR << "Signal was smaller than 0. There is an error in the input image file.";
          throw mitk::pa::InvalidValueException("Signal was smaller than 0. There is an error in the input image file.");
        }
        image->SetData(log10(image->GetData(x, y, z)), x, y, z);
      }
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
