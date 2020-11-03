/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkImageSliceSelectionFilter.h"

mitk::ImageSliceSelectionFilter::ImageSliceSelectionFilter()
{
  MITK_INFO << "Instantiating CropImageFilter...";
  SetNumberOfIndexedInputs(1);
  SetNumberOfIndexedOutputs(1);
  MITK_INFO << "Instantiating CropImageFilter...[Done]";
}

mitk::ImageSliceSelectionFilter::~ImageSliceSelectionFilter()
{
  MITK_INFO << "Destructed CastToFloatImageFilter.";
}

void mitk::ImageSliceSelectionFilter::GenerateData()
{
}
