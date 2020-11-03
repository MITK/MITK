/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkPAVolume.h"
#include <mitkImageWriteAccessor.h>
#include <mitkImageReadAccessor.h>
#include <mutex>

mitk::pa::Volume::Volume(double* data,
  unsigned int xDim, unsigned int yDim, unsigned int zDim, double spacing)
{
  MITK_INFO << "Initialized by data*";
  if (data == nullptr)
    mitkThrow() << "You may not initialize a mitk::Volume with a nullptr";
  m_InternalMitkImage = mitk::Image::New();

  auto* dimensions = new unsigned int[NUMBER_OF_SPATIAL_DIMENSIONS];
  dimensions[0] = yDim;
  dimensions[1] = xDim;
  dimensions[2] = zDim;
  m_XDim = xDim;
  m_YDim = yDim;
  m_ZDim = zDim;
  mitk::PixelType pixelType = mitk::MakeScalarPixelType<double>();

  m_InternalMitkImage->Initialize(pixelType, NUMBER_OF_SPATIAL_DIMENSIONS, dimensions);
  m_InternalMitkImage->SetImportVolume(data, Image::ImportMemoryManagementType::CopyMemory);

  SetSpacing(spacing);

  m_FastAccessDataPointer = GetData();

  delete data;
}

mitk::pa::Volume::Volume(mitk::Image::Pointer image)
{
  MITK_INFO << "Initialized by mitk::Image";

  if (image.IsNull())
    mitkThrow() << "You may not initialize a mitk::Volume with a null reference to an mitk image";

  unsigned int* dimensions = image->GetDimensions();
  m_YDim = dimensions[0];
  m_XDim = dimensions[1];
  m_ZDim = dimensions[2];

  m_InternalMitkImage = image;

  m_FastAccessDataPointer = GetData();
}

double mitk::pa::Volume::GetSpacing()
{
  return m_InternalMitkImage->GetGeometry()->GetSpacing()[0];
}

void mitk::pa::Volume::SetSpacing(double spacing)
{
  const mitk::ScalarType spacingArray[]{ spacing, spacing, spacing };
  m_InternalMitkImage->SetSpacing(spacingArray);
}

mitk::pa::Volume::~Volume()
{
  m_InternalMitkImage = nullptr;
}

mitk::pa::Volume::Pointer mitk::pa::Volume::New(double* data, unsigned int xDim, unsigned int yDim, unsigned int zDim, double spacing)
{
  mitk::pa::Volume::Pointer smartPtr = new mitk::pa::Volume(data, xDim, yDim, zDim, spacing);
  smartPtr->UnRegister();
  return smartPtr;
}

mitk::pa::Volume::Pointer mitk::pa::Volume::New(mitk::Image::Pointer image)
{
  mitk::pa::Volume::Pointer smartPtr = new mitk::pa::Volume(image);
  smartPtr->UnRegister();
  return smartPtr;
}

mitk::Image::Pointer mitk::pa::Volume::AsMitkImage()
{
  return m_InternalMitkImage;
}

mitk::pa::Volume::Pointer mitk::pa::Volume::DeepCopy()
{
  long length = GetXDim()*GetYDim()*GetZDim();
  auto* data = new double[length];
  memcpy(data, GetData(), length * sizeof(double));

  return mitk::pa::Volume::New(data, GetXDim(), GetYDim(), GetZDim(), GetSpacing());
}

double mitk::pa::Volume::GetData(unsigned int x, unsigned int y, unsigned int z)
{
  return m_FastAccessDataPointer[GetIndex(x, y, z)];
}

void mitk::pa::Volume::SetData(double data, unsigned int x, unsigned int y, unsigned int z)
{
  m_FastAccessDataPointer[GetIndex(x, y, z)] = data;
}

unsigned int mitk::pa::Volume::GetXDim()
{
  return m_XDim;
}

unsigned int mitk::pa::Volume::GetYDim()
{
  return m_YDim;
}

unsigned int mitk::pa::Volume::GetZDim()
{
  return m_ZDim;
}

double* mitk::pa::Volume::GetData() const
{
  mitk::ImageWriteAccessor imgRead(m_InternalMitkImage, m_InternalMitkImage->GetVolumeData());
  return (double*)imgRead.GetData();
}

long long mitk::pa::Volume::GetIndex(unsigned int x, unsigned int y, unsigned int z)
{
#ifdef _DEBUG

  if (x < 0 || x >(GetXDim() - 1) || y < 0 || y >(GetYDim() - 1) || z < 0 || z >(GetZDim() - 1))
  {
    MITK_ERROR << "Index out of bounds at " << x << "|" << y << "|" << z;
    mitkThrow() << "Index out of bounds exception!";
  }

#endif
  return ((long long)z) * m_XDim * m_YDim + ((long long)x) * m_YDim + ((long long)y);
}
