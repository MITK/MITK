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
#include <vector>

//vtkStandardNewMacro(mitk::ImageVtkAccessor);

mitk::ImageVtkAccessor* mitk::ImageVtkAccessor::New(ImagePointer iP, const ImageDataItem* iDI)
  {

    vtkObject* ret = vtkObjectFactory::CreateInstance("ImageVtkAccessor");
      if(ret)
      {
      return static_cast<mitk::ImageVtkAccessor*>(ret);
      }

    ImageDataItem* noConst_iDI = const_cast<ImageDataItem*>(iDI);

    return new ImageVtkAccessor(iP, noConst_iDI);
  }

mitk::ImageVtkAccessor::ImageVtkAccessor(
      mitk::ImagePointer iP,
      mitk::ImageDataItem* iDI
      ) :
    ImageAccessorBase(NULL, iDI),
    vtkImageData()
    {

      m_Image = iP.GetPointer();

      m_Image->m_VtkReadersLock.Lock();

      m_Image->m_VtkReaders.push_back(this);
      //printf("m_VtkReaders.size(): %d\n", (int) m_Image->m_VtkReaders.size());

      m_Image->m_VtkReadersLock.Unlock();

    }
