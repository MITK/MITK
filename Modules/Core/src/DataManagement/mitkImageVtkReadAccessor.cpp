/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkImageVtkReadAccessor.h"

#include "mitkImage.h"

#include <vtkImageData.h>

const mitk::Image *mitk::ImageVtkReadAccessor::GetImage() const
{
  return m_Image;
}

mitk::ImageVtkReadAccessor::ImageVtkReadAccessor(ImageConstPointer iP,
                                                 const mitk::ImageDataItem *iDI,
                                                 const vtkImageData *imageDataVtk)
  : ImageAccessorBase(iP, iDI), m_Image(iP.GetPointer()), m_ImageDataVtk(imageDataVtk)
{
  m_Image->m_VtkReadersLock.lock();

  m_Image->m_VtkReaders.push_back(this);
  // printf("m_VtkReaders.size(): %d\n", (int) m_Image->m_VtkReaders.size());

  m_Image->m_VtkReadersLock.unlock();
}

mitk::ImageVtkReadAccessor::~ImageVtkReadAccessor()
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

const vtkImageData *mitk::ImageVtkReadAccessor::GetVtkImageData() const
{
  return m_ImageDataVtk;
}
