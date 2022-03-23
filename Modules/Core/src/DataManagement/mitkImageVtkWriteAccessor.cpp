/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkImageVtkWriteAccessor.h"

#include "mitkImage.h"

#include <vtkImageData.h>

const mitk::Image *mitk::ImageVtkWriteAccessor::GetImage() const
{
  return m_Image;
}

mitk::ImageVtkWriteAccessor::ImageVtkWriteAccessor(ImagePointer iP,
                                                   const mitk::ImageDataItem *iDI,
                                                   vtkImageData *imageDataVtk)
  : ImageAccessorBase(nullptr, iDI), m_Image(iP.GetPointer()), m_ImageDataVtk(imageDataVtk)
{
  m_Image->m_VtkReadersLock.lock();

  m_Image->m_VtkReaders.push_back(this);
  // printf("m_VtkReaders.size(): %d\n", (int) m_Image->m_VtkReaders.size());

  m_Image->m_VtkReadersLock.unlock();
}

mitk::ImageVtkWriteAccessor::~ImageVtkWriteAccessor()
{
  m_Image->m_VtkReadersLock.lock();

  auto it = std::find(m_Image->m_VtkReaders.begin(), m_Image->m_VtkReaders.end(), this);
  if (it != m_Image->m_VtkReaders.end())
  {
    m_Image->m_VtkReaders.erase(it);
  }
  // printf("m_VtkReaders.size(): %d\n", (int) m_Image->m_VtkReaders.size());

  m_Image->m_VtkReadersLock.unlock();
}

vtkImageData *mitk::ImageVtkWriteAccessor::GetVtkImageData() const
{
  return m_ImageDataVtk;
}
