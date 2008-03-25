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

#ifndef __itkImportMitkImageContainer_h
#define __itkImportMitkImageContainer_h

#include <itkImportImageContainer.h>
#include <mitkImageDataItem.h>

namespace itk
{

/** \class ImportMitkImageContainer
 * Defines an itk::Image front-end to an mitk::Image. This container
 * conforms to the ImageContainerInterface. This is a full-fleged Object,
 * so there is modification time, debug, and reference count information.
 *
 * Template parameters for ImportMitkImageContainer:
 *
 * TElementIdentifier =
 *     An INTEGRAL type for use in indexing the imported buffer.
 *
 * TElement =
 *    The element type stored in the container.
 */
  
template <typename TElementIdentifier, typename TElement>
class ImportMitkImageContainer:  public ImportImageContainer<TElementIdentifier, TElement>
{
public:
  /** Standard class typedefs. */
  typedef ImportMitkImageContainer     Self;
  typedef Object  Superclass;
  typedef SmartPointer<Self>  Pointer;
  typedef SmartPointer<const Self>  ConstPointer;
    
  /** Save the template parameters. */
  typedef TElementIdentifier  ElementIdentifier;
  typedef TElement            Element;
    
  /** Method for creation through the object factory. */
  itkNewMacro(Self);
  
  /** Standard part of every itk Object. */
  itkTypeMacro(ImportMitkImageContainer, ImportImageContainer);

  ///** Get the pointer from which the image data is imported. */
  //TElement *GetImportPointer() {return m_ImportPointer;};

  /** \brief Set the mitk::ImageDataItem to be imported  */
  void SetImageDataItem(mitk::ImageDataItem* imageDataItem);

protected:
  ImportMitkImageContainer();
  virtual ~ImportMitkImageContainer();

  /** PrintSelf routine. Normally this is a protected internal method. It is
   * made public here so that Image can call this method.  Users should not
   * call this method but should call Print() instead. */
  void PrintSelf(std::ostream& os, Indent indent) const;

  TElement* AllocateElements(ElementIdentifier size) const;
private:
  ImportMitkImageContainer(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented

  mitk::ImageDataItem::Pointer m_ImageDataItem;
};

} // end namespace itk

// Define instantiation macro for this template.
#define ITK_TEMPLATE_ImportMitkImageContainer(_, EXPORT, x, y) namespace itk { \
  _(2(class EXPORT ImportMitkImageContainer< ITK_TEMPLATE_2 x >)) \
  namespace Templates { typedef ImportMitkImageContainer< ITK_TEMPLATE_2 x > ImportMitkImageContainer##y; } \
  }

//#if ITK_TEMPLATE_EXPLICIT
//# include "Templates/itkImportMitkImageContainer+-.h"
//#endif

#if ITK_TEMPLATE_TXX
# include "itkImportMitkImageContainer.txx"
#endif

#endif
