/*===================================================================
mitkImageSliceSelectionFilter
The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

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
