/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkCropImageFilter.h"
#include <mitkImageReadAccessor.h>
#include <mitkImageWriteAccessor.h>

mitk::CropImageFilter::CropImageFilter() :
  m_XPixelsCropStart(0),
  m_YPixelsCropStart(0),
  m_ZPixelsCropStart(0),
  m_XPixelsCropEnd(0),
  m_YPixelsCropEnd(0),
  m_ZPixelsCropEnd(0)
{
  MITK_INFO << "Instantiating CropImageFilter...";
  SetNumberOfIndexedInputs(1);
  SetNumberOfIndexedOutputs(1);
  MITK_INFO << "Instantiating CropImageFilter...[Done]";
}

mitk::CropImageFilter::~CropImageFilter()
{
  MITK_INFO << "Destructed CastToFloatImageFilter.";
}

void mitk::CropImageFilter::SanityCheckPreconditions()
{
  mitk::Image::Pointer inputImage = GetInput();

  std::string type = inputImage->GetPixelType().GetTypeAsString();
  if (!(type == "scalar (float)" || type == " (float)"))
  {
    MITK_ERROR << "This filter can currently only handle float type images.";
    mitkThrow() << "This filter can currently only handle float type images.";
  }

  if (m_XPixelsCropStart + m_XPixelsCropEnd >= inputImage->GetDimension(0))
  {
    MITK_ERROR << "X Crop area too large for selected input image";
    mitkThrow() << "X Crop area too large for selected input image";
  }
  if (m_YPixelsCropStart + m_YPixelsCropEnd >= inputImage->GetDimension(1))
  {
    MITK_ERROR << "Y Crop area too large for selected input image";
    mitkThrow() << "Y Crop area too large for selected input image";
  }

  if (inputImage->GetDimension() == 3)
  {
    if (m_ZPixelsCropStart + m_ZPixelsCropEnd >= inputImage->GetDimension(2))
    {
      MITK_INFO << m_ZPixelsCropStart + m_ZPixelsCropEnd << "vs" << inputImage->GetDimension(2);
      MITK_ERROR << "Z Crop area too large for selected input image";
      mitkThrow() << "Z Crop area too large for selected input image";
    }
  }
  else
  {
    if (m_ZPixelsCropStart + m_ZPixelsCropEnd > 0)
    {
      mitkThrow() << "Z Crop area too large for selected input image";
    }
  }
}

void mitk::CropImageFilter::GenerateData()
{
  mitk::Image::Pointer inputImage = GetInput();
  mitk::Image::Pointer outputImage = GetOutput();

  SanityCheckPreconditions();
  unsigned int* outputDim;
  unsigned int* inputDim = inputImage->GetDimensions();
  if (inputImage->GetDimension() == 2)
    outputDim = new unsigned int[2]{ inputImage->GetDimension(0) - m_XPixelsCropStart - m_XPixelsCropEnd,
    inputImage->GetDimension(1) - m_YPixelsCropStart - m_YPixelsCropEnd };
  else
    outputDim = new unsigned int[3]{ inputImage->GetDimension(0) - m_XPixelsCropStart - m_XPixelsCropEnd,
    inputImage->GetDimension(1) - m_YPixelsCropStart - m_YPixelsCropEnd,
    inputImage->GetDimension(2) - m_ZPixelsCropStart - m_ZPixelsCropEnd };

  outputImage->Initialize(mitk::MakeScalarPixelType<float>(), inputImage->GetDimension(), outputDim);
  outputImage->SetSpacing(inputImage->GetGeometry()->GetSpacing());

  ImageReadAccessor accR(inputImage);
  const float* inputData = (const float*)(accR.GetData());

  ImageWriteAccessor accW(outputImage);
  float* outputData = (float*)(accW.GetData());

  unsigned int zLength = inputImage->GetDimension() == 3 ? outputDim[2] : 1;

  for (unsigned int sl = 0; sl < zLength; ++sl)
  {
    for (unsigned int l = 0; l < outputDim[0]; ++l)
    {
      for (unsigned int s = 0; s < outputDim[1]; ++s)
      {
        outputData[l + s*outputDim[0] + sl*outputDim[0] * outputDim[1]] =
          inputData[(l + m_XPixelsCropStart) + (s + m_YPixelsCropStart)*inputDim[0] +
          (sl + m_ZPixelsCropStart)*inputDim[0] * inputDim[1]];
      }
    }
  }
  delete[] outputDim;
}
