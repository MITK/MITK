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

}

  
template <typename TElementIdentifier, typename TElement>
ImportMitkImageContainer< TElementIdentifier , TElement >
::~ImportMitkImageContainer()
{
  m_ImageDataItem = NULL;
}

template <typename TElementIdentifier, typename TElement>
void
ImportMitkImageContainer< TElementIdentifier , TElement >
::SetImageDataItem(mitk::ImageDataItem* imageDataItem)
{
  m_ImageDataItem = imageDataItem;
  
  SetImportPointer( (TElement*) m_ImageDataItem->GetData(), m_ImageDataItem->GetSize(), false);

  this->Modified();
}

template <typename TElementIdentifier, typename TElement>
void
ImportMitkImageContainer< TElementIdentifier , TElement >
::PrintSelf(std::ostream& os, Indent indent) const
{
  Superclass::PrintSelf(os,indent);

  os << indent << "ImageDataItem: " << m_ImageDataItem << std::endl;
}

} // end namespace itk

#endif
