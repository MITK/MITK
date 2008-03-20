/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: 13129 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef _itkImportMitkImageContainer_txx
#define _itkImportMitkImageContainer_txx

#include "itkImportMitkImageContainer.h"

namespace itk
{

template <typename TElementIdentifier, typename TElement>
ImportMitkImageContainer<TElementIdentifier , TElement>
::ImportMitkImageContainer() : ImportImageContainer()
{
//  m_ImportPointer = 0;
//  m_Capacity = 0;
//  m_Size = 0;
  m_ContainerManageMemory = false;
}

  
template <typename TElementIdentifier, typename TElement>
ImportMitkImageContainer< TElementIdentifier , TElement >
::~ImportMitkImageContainer()
{
  if (m_ImageDataItem.IsNotNull())
  {
    m_ImageDataItem = NULL;
    m_ImportPointer = NULL;
  }
  if (m_ImportPointer)
    {
    delete [] m_ImportPointer;
    }
}


/**
 * Tell the container to allocate enough memory to allow at least
 * as many elements as the size given to be stored.  
 */
template <typename TElementIdentifier, typename TElement>
void
ImportMitkImageContainer< TElementIdentifier , TElement >
::Reserve(ElementIdentifier size)
{
  if (m_ImageDataItem.IsNotNull())
  {
    m_ImageDataItem = NULL;
    m_ImportPointer = NULL;
  }
  if (m_ImportPointer)
    {
    if (size > m_Capacity)
      {
      TElement* temp = this->AllocateElements(size);
      // only copy the portion of the data used in the old buffer
      memcpy(temp, m_ImportPointer, m_Size*sizeof(TElement));

      delete [] m_ImportPointer;

      m_ImportPointer = temp;
      m_Capacity = size;
      m_Size = size;
      this->Modified();
      }
    }
  else
    {
    m_ImportPointer = this->AllocateElements(size);
    m_Capacity = size;
    m_Size = size;
    this->Modified();
    }
}


/**
 * Tell the container to try to minimize its memory usage for storage of
 * the current number of elements.  
 */
template <typename TElementIdentifier, typename TElement>
void
ImportMitkImageContainer< TElementIdentifier , TElement >
::Squeeze(void)
{
  if (m_ImageDataItem.IsNull() && (m_ImportPointer))
    {
    if (m_Size < m_Capacity)
      {
      TElement* temp = this->AllocateElements(m_Size);
      memcpy(temp, m_ImportPointer, m_Size*sizeof(TElement));
      delete [] m_ImportPointer;
      m_ImportPointer = temp;
      m_ContainerManageMemory = true;
      m_Capacity = m_Size;

      this->Modified();
      }
    }
}


/**
 * Tell the container to try to minimize its memory usage for storage of
 * the current number of elements.  
 */
template <typename TElementIdentifier, typename TElement>
void
ImportMitkImageContainer< TElementIdentifier , TElement >
::Initialize(void)
{
  if (m_ImageDataItem.IsNotNull())
  {
    m_ImageDataItem = NULL;
    m_ImportPointer = NULL;
  }
  if (m_ImportPointer)
    {
    m_ImportPointer = 0;
    m_Capacity = 0;
    m_Size = 0;
    
    this->Modified();
    }
}


/**
 * Set the pointer from which the image data is imported.  "num" is
 * the number of pixels in the block of memory. If
 * "LetContainerManageMemory" is false, then the application retains
 * the responsibility of freeing the memory for this image data.  If
 * "LetContainerManageMemory" is true, then this class will free the
 * memory when this object is destroyed.
 */
template <typename TElementIdentifier, typename TElement>
void
ImportMitkImageContainer< TElementIdentifier , TElement >
::SetImageDataItem(mitk::ImageDataItem* imageDataItem)
{
  if (m_ImageDataItem.IsNull() && (m_ImportPointer))
  {
    delete [] m_ImportPointer;
    m_ImportPointer = NULL;
  }
  m_ImageDataItem = imageDataItem;
  if(imageDataItem != NULL)
  {
    m_ImportPointer = imageDataItem->GetData();
  }
  
  m_Capacity = 0;
  m_Size = 0;

  this->Modified();
}

template <typename TElementIdentifier, typename TElement>
TElement* ImportMitkImageContainer< TElementIdentifier , TElement >
::AllocateElements(ElementIdentifier size) const
{
  // Encapsulate all image memory allocation here to throw an
  // exception when memory allocation fails even when the compiler
  // does not do this by default.
  TElement* data;
  try
    {
    data = new TElement[size];
    }
  catch(...)
    {
    data = 0;
    }
  if(!data)
    {
    // We cannot construct an error string here because we may be out
    // of memory.  Do not use the exception macro.
    throw MemoryAllocationError(__FILE__, __LINE__,
                                "Failed to allocate memory for image.",
                                ITK_LOCATION);
    }
  return data;
}

template <typename TElementIdentifier, typename TElement>
void
ImportMitkImageContainer< TElementIdentifier , TElement >
::PrintSelf(std::ostream& os, Indent indent) const
{
  Superclass::PrintSelf(os,indent);

  os << indent << "ImageDataItem: " << m_ImageDataItem << std::endl;
  os << indent << "Pointer: " << &m_ImportPointer << std::endl;
  os << indent << "Size: " << m_Size << std::endl;
  os << indent << "Capacity: " << m_Capacity << std::endl;
}

} // end namespace itk

#endif
