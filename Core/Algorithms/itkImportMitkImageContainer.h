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
class ImportMitkImageContainer:  public Object
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
  itkTypeMacro(ImportMitkImageContainer, Object);

  /** Get the pointer from which the image data is imported. */
  TElement *GetImportPointer() {return m_ImportPointer;};

  /** \brief Set the mitk::ImageDataItem to be imported  */
  void SetImageDataItem(mitk::ImageDataItem* imageDataItem);

  /** Index operator. This version can be an lvalue. */
  TElement & operator[](const ElementIdentifier id)
    { return m_ImportPointer[id]; };

  /** Index operator. This version can only be an rvalue */
  const TElement & operator[](const ElementIdentifier id) const
    { return m_ImportPointer[id]; };

  /** Return a pointer to the beginning of the buffer.  This is used by
   * the image iterator class. */
  TElement *GetBufferPointer()
    { return m_ImportPointer; };
  
  /** Get the number of elements currently stored in the container. */
  unsigned long Size(void) const
    { return (unsigned long) m_Size; };

  /** Tell the container to allocate enough memory to allow at least
   * as many elements as the size given to be stored.  If new memory
   * needs to be allocated, the contents of the old buffer are copied
   * to the new area.  The old buffer is deleted if the original pointer
   * was passed in using "LetContainerManageMemory"=true. The new buffer's
   * memory management will be handled by the container from that point on.
   *
   * \sa SetImportPointer() */
  void Reserve(ElementIdentifier num);
  
  /** Tell the container to try to minimize its memory usage for
   * storage of the current number of elements.  If new memory is
   * allocated, the contents of old buffer are copied to the new area.
   * The previous buffer is deleted if the original pointer was in
   * using "LetContainerManageMemory"=true.  The new buffer's memory
   * management will be handled by the container from that point on. */
  void Squeeze(void);
  
  /** Tell the container to release any of its allocated memory. */
  void Initialize(void);

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

  TElement            *m_ImportPointer;
  TElementIdentifier   m_Size;
  TElementIdentifier   m_Capacity;

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
