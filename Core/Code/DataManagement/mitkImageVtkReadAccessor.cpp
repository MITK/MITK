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

#include "mitkImageVtkReadAccessor.h"

#include "mitkImage.h"

#include <vtkImageData.h>

const mitk::Image* mitk::ImageVtkReadAccessor::GetImage() const
{
  return m_Image;
}

mitk::ImageVtkReadAccessor::ImageVtkReadAccessor(
    ImageConstPointer iP, const mitk::ImageDataItem* iDI,
    const vtkImageData* imageDataVtk)
  : ImageAccessorBase(iP, iDI)
  , m_Image(iP.GetPointer())
  , m_ImageDataVtk(imageDataVtk)
{
  m_Image->m_VtkReadersLock.Lock();

  m_Image->m_VtkReaders.push_back(this);
  //printf("m_VtkReaders.size(): %d\n", (int) m_Image->m_VtkReaders.size());

  m_Image->m_VtkReadersLock.Unlock();
}

mitk::ImageVtkReadAccessor::~ImageVtkReadAccessor()
{
  m_Image->m_VtkReadersLock.Lock();

  std::vector<ImageAccessorBase*>::iterator it =
      std::find(m_Image->m_VtkReaders.begin(), m_Image->m_VtkReaders.end(), this);
  if (it != m_Image->m_VtkReaders.end())
  {
    m_Image->m_VtkReaders.erase(it);
  }
  //printf("m_VtkReaders.size(): %d\n", (int) m_Image->m_VtkReaders.size());

  m_Image->m_VtkReadersLock.Unlock();
}

const vtkImageData*mitk::ImageVtkReadAccessor::GetVtkImageData() const
{
  return m_ImageDataVtk;
}
