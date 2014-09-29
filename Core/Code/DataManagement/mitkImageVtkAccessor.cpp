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

#include "mitkImageVtkAccessor.h"

#include "mitkImage.h"

mitk::ImageVtkAccessor::ImageVtkAccessor(ImagePointer iP, const mitk::ImageDataItem* iDI)
  : mitk::ImageVtkWriteAccessor(iP, iDI, NULL)
{
}

mitk::ImageVtkAccessor* mitk::ImageVtkAccessor::New(ImagePointer iP, const mitk::ImageDataItem* iDI)
{
  return new ImageVtkAccessor(iP, iDI);
}
