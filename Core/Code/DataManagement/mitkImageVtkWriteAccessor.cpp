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

#include "mitkImageVtkWriteAccessor.h"

#include "mitkImage.h"

#include <vtkImageData.h>

const mitk::Image* mitk::ImageVtkWriteAccessor::GetImage() const
{
  return m_Image;
}

mitk::ImageVtkWriteAccessor::ImageVtkWriteAccessor(ImagePointer iP, const mitk::ImageDataItem* iDI, vtkImageData* imageDataVtk)
  : ImageAccessorBase(NULL, iDI)
  , m_Image(iP.GetPointer())
  , m_ImageDataVtk(imageDataVtk)
{
  m_Image->m_VtkReadersLock.Lock();

  m_Image->m_VtkReaders.push_back(this);
  //printf("m_VtkReaders.size(): %d\n", (int) m_Image->m_VtkReaders.size());

  m_Image->m_VtkReadersLock.Unlock();
}

mitk::ImageVtkWriteAccessor::~ImageVtkWriteAccessor()
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

vtkImageData* mitk::ImageVtkWriteAccessor::GetVtkImageData() const
{
  return m_ImageDataVtk;
}
