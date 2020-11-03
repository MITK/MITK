/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkCastToFloatImageFilter.h"
#include "mitkImageReadAccessor.h"
#include "mitkImageWriteAccessor.h"

mitk::CastToFloatImageFilter::CastToFloatImageFilter()
{
  MITK_INFO << "Instantiating CastToFloatImageFilter...";
  SetNumberOfIndexedInputs(1);
  SetNumberOfIndexedOutputs(1);
  MITK_INFO << "Instantiating CastToFloatImageFilter...[Done]";
}

mitk::CastToFloatImageFilter::~CastToFloatImageFilter()
{
  MITK_INFO << "Destructed CastToFloatImageFilter.";
}

template<class TPixelType>
float* CastData(const void* inputArray, unsigned long length)
{
  float* outputArray = new float[length];
  for (unsigned long i = 0; i < length; ++i)
  {
    outputArray[i] = (float)((TPixelType*)inputArray)[i];
  }
  return outputArray;
}

void mitk::CastToFloatImageFilter::GenerateData()
{
  mitk::Image::Pointer inputImage = GetInput();
  mitk::Image::Pointer outputImage = GetOutput();

  std::string type = inputImage->GetPixelType().GetTypeAsString();

  if (type == "scalar (float)" || type == " (float)")
  {
    outputImage->Initialize(mitk::MakeScalarPixelType<float>(), inputImage->GetDimension(), inputImage->GetDimensions());
    outputImage->SetSpacing(inputImage->GetGeometry()->GetSpacing());
    outputImage->SetImportVolume(mitk::ImageWriteAccessor(inputImage).GetData(), 0, 0, mitk::Image::ImportMemoryManagementType::CopyMemory);
    MITK_INFO << "Input is already float type. Nothing to do here.";
    return;
  }
  if (outputImage.IsNull())
  {
    outputImage = mitk::Image::New();
  }

  unsigned long length = 1;
  for (unsigned int i = 0, limit = inputImage->GetDimension(); i < limit; ++i)
    length *= inputImage->GetDimensions()[i];

  float* outputData;

  mitk::ImageReadAccessor readAccess(inputImage);

  if (type == "scalar (short)" || type == " (short)")
    outputData = CastData<short>(readAccess.GetData(), length);
  else if (type == "scalar (unsigned short)" || type == " (unsigned short)" || type == " (unsigned_short)" || type == "scalar (unsigned_short)")
    outputData = CastData<unsigned short>(readAccess.GetData(), length);
  else if (type == "scalar (int)" || type == " (int)")
    outputData = CastData<int>(readAccess.GetData(), length);
  else if (type == "scalar (unsigned int)" || type == " (unsigned int)" || type == " (unsigned_int)" || type == "scalar (unsigned_int)")
    outputData = CastData<unsigned int>(readAccess.GetData(), length);
  else if (type == "scalar (double)" || type == " (double)")
    outputData = CastData<double>(readAccess.GetData(), length);
  else if (type == "scalar (long)" || type == " (long)")
    outputData = CastData<long>(readAccess.GetData(), length);
  else
    mitkThrow() << "The given image has a datatype that is not yet supported. Given datatype: " << type;

  outputImage->Initialize(mitk::MakeScalarPixelType<float>(), inputImage->GetDimension(), inputImage->GetDimensions());
  outputImage->SetSpacing(inputImage->GetGeometry()->GetSpacing());
  outputImage->SetImportVolume(outputData, 0, 0, mitk::Image::ImportMemoryManagementType::CopyMemory);
  delete[] outputData;
}
