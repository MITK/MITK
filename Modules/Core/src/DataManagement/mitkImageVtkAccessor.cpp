/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkImageVtkAccessor.h"

#include "mitkImage.h"

mitk::ImageVtkAccessor::ImageVtkAccessor(ImagePointer iP, const mitk::ImageDataItem *iDI)
  : mitk::ImageVtkWriteAccessor(iP, iDI, nullptr)
{
}

mitk::ImageVtkAccessor *mitk::ImageVtkAccessor::New(ImagePointer iP, const mitk::ImageDataItem *iDI)
{
  return new ImageVtkAccessor(iP, iDI);
}
