/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef _itkImportMitkImageContainer_txx
#define _itkImportMitkImageContainer_txx

#include "itkImportMitkImageContainer.h"

namespace itk
{
  template <typename TElementIdentifier, typename TElement>
  ImportMitkImageContainer<TElementIdentifier, TElement>::ImportMitkImageContainer() : m_imageAccess(nullptr)
  {
  }

  template <typename TElementIdentifier, typename TElement>
  ImportMitkImageContainer<TElementIdentifier, TElement>::~ImportMitkImageContainer()
  {
    if (m_imageAccess != nullptr)
      delete m_imageAccess;
    m_imageAccess = nullptr;
  }

  /*
  template <typename TElementIdentifier, typename TElement>
  void
  ImportMitkImageContainer< TElementIdentifier , TElement >
  ::SetImageDataItem(mitk::ImageDataItem* imageDataItem)
  {
    m_ImageDataItem = imageDataItem;

    this->SetImportPointer( (TElement*) m_ImageDataItem->GetData(), m_ImageDataItem->GetSize()/sizeof(Element), false);

    this->Modified();
  }
  */

  template <typename TElementIdentifier, typename TElement>
  void ImportMitkImageContainer<TElementIdentifier, TElement>::SetImageAccessor(mitk::ImageAccessorBase *imageAccess,
                                                                                size_t noOfBytes)
  {
    m_imageAccess = imageAccess;

    this->SetImportPointer((TElement *)m_imageAccess->GetData(), noOfBytes / sizeof(Element), false);

    this->Modified();
  }

  template <typename TElementIdentifier, typename TElement>
  void ImportMitkImageContainer<TElementIdentifier, TElement>::PrintSelf(std::ostream &os, Indent indent) const
  {
    Superclass::PrintSelf(os, indent);

    os << indent << "ImageAccessor: " << m_imageAccess << std::endl;
  }

} // end namespace itk

#endif
