#ifndef __itkSmartPointerVectorContainer_h
#define __itkSmartPointerVectorContainer_h
/*

   Defines a vector container just like from itkSmartPointerVectorContainer.h.
   The important difference ist, that this vector holds SmartPointers 
   to object instead of objects themselves.

   Major difference to SmartPointerVectorContainer<SmartPointer<Object>>:

   When you ask for a ConstIterator you won't get const SmartPointers,
   but a list of const Object*. 
*/


/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "itkObject.h"

#include <utility>
#include <vector>

namespace itk
{

/** \class SmartPointerVectorContainer
 * Define a front-end to the STL "vector" container that conforms to the
 * IndexedContainerInterface.  This is a full-fleged Object, so
 * there is modification time, debug, and reference count information.
 *
 * Template parameters for SmartPointerVectorContainer:
 *
 * TElementIdentifier =
 *     An INTEGRAL type for use in indexing the vector.
 *
 * TElement =
 *    The element type stored in the container.
 *
 * \ingroup DataRepresentation
 */
template <
  typename TElementIdentifier,
  typename TElement
  >
class SmartPointerVectorContainer: 
  public Object,
  public std::vector<itk::SmartPointer<TElement> >  /** difference: we handle smart pointers */
{
public:
  /** Standard class typedefs. */
  typedef SmartPointerVectorContainer Self;
  typedef Object                      Superclass;
  typedef SmartPointer<Self>          Pointer;
  typedef SmartPointer<const Self>    ConstPointer;
  
  /** Save the template parameters. */
  typedef TElementIdentifier                     ElementIdentifier;
  typedef TElement                               Element;
  typedef const TElement                         ConstElement;
  typedef itk::SmartPointer<Element>             ElementPointer;
  typedef const itk::SmartPointer<const Element> ConstElementPointer;
  
private:
  /** Quick access to the STL vector type that was inherited. */
  typedef std::vector<ElementPointer>             VectorType;
  typedef typename VectorType::size_type          size_type;  
  typedef typename VectorType::iterator           VectorIterator;
  typedef typename VectorType::const_iterator     VectorConstIterator;
    
protected:
  /** Provide pass-through constructors corresponding to all the STL
   * vector constructors.  These are for internal use only since this is also
   * an Object which must be constructed through the "New()" routine. */
  SmartPointerVectorContainer():
    Object(), VectorType() {}
  SmartPointerVectorContainer(size_type n):
    Object(), VectorType(n) {}
  SmartPointerVectorContainer(size_type n, const Element& x):
    Object(), VectorType(n, x) {}
  SmartPointerVectorContainer(const Self& r):
    Object(), VectorType(r) {}
  template <typename InputIterator>
  SmartPointerVectorContainer(InputIterator first, InputIterator last):
    Object(), VectorType(first, last) {}
  
public:

  /** This type is provided to Adapt this container as an STL container */
  typedef VectorType STLContainerType;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);
  
  /** Standard part of every itk Object. */
  itkTypeMacro(SmartPointerVectorContainer, Object);

  /** Convenient typedefs for the iterator and const iterator. */
  class Iterator;
  class ConstIterator;
    
  /** Cast the container to a STL container type */
  STLContainerType & CastToSTLContainer() {
     return dynamic_cast<STLContainerType &>(*this); }

  /** Cast the container to a const STL container type */
  const STLContainerType & CastToSTLConstContainer() const {
     return dynamic_cast<const STLContainerType &>(*this); }

  /** Friends to this class. */
  friend class Iterator;
  friend class ConstIterator;
  
  class Iterator
  {
  public:
    Iterator() {}
    Iterator(size_type d, const VectorType* vec, const VectorIterator& i): m_Pos(d), m_Vector(vec), m_Iter(i) {}
    
    ElementPointer& operator* ()    { return *m_Iter; }
    Element*        operator-> ()   { return m_Iter == m_Vector->end() ? NULL : m_Iter->GetPointer(); }
    ElementPointer& operator++ ()   { ++m_Pos; ++m_Iter; return *m_Iter; }
    ElementPointer  operator++ (int) { Iterator temp(*this); ++m_Pos; ++m_Iter; return *temp; }
    ElementPointer& operator-- ()   { --m_Pos; --m_Iter; return *this; }
    ElementPointer  operator-- (int) { Iterator temp(*this); --m_Pos; --m_Iter; return *temp; }

    bool operator == (const Iterator& r) const { return m_Iter == r.m_Iter; }
    bool operator != (const Iterator& r) const { return m_Iter != r.m_Iter; }
    bool operator == (const ConstIterator& r) const { return m_Iter == r.m_Iter; }
    bool operator != (const ConstIterator& r) const { return m_Iter != r.m_Iter; }
    
    /** Get the index into the SmartPointerVectorContainer associated with this iterator.   */
    ElementIdentifier Index(void) const { return static_cast<ElementIdentifier>( m_Pos ); }
    
    /** Get the value at this iterator's location in the SmartPointerVectorContainer.   */
    Element* Value(void) const { return m_Iter->GetPointer(); }
    
  private:
    size_type m_Pos;
    const VectorType* m_Vector;
    VectorIterator m_Iter;
    friend class ConstIterator;
  };
  
  class ConstIterator
  {
  public:
    ConstIterator() {}
    ConstIterator(size_type d, const VectorType* vec, const VectorConstIterator& i): m_Pos(d), m_Vector(vec), m_Iter(i) {}
    ConstIterator(const Iterator& r): m_Pos(r.m_Pos), m_Vector(r.m_Vector), m_Iter(r.m_Iter) {}
    
    ConstElementPointer operator* ()    { return ConstElementPointer(m_Iter == m_Vector->end() ? NULL : m_Iter->GetPointer()); } 
    ConstElement*       operator-> ()   { return m_Iter == m_Vector->end() ? NULL : m_Iter->GetPointer(); }
    ConstElementPointer operator++ ()   { ++m_Pos; ++m_Iter; return ConstElementPointer(m_Iter == m_Vector->end() ? NULL : m_Iter->GetPointer()); }
    ConstElementPointer operator++ (int) { ConstIterator temp(*this); ++m_Pos; ++m_Iter; return *temp; }
    ConstElementPointer operator-- ()   { --m_Pos; --m_Iter; return ConstElementPointer(m_Iter == m_Vector->end() ? NULL : m_Iter->GetPointer()); }
    ConstElementPointer operator-- (int) { ConstIterator temp(*this); --m_Pos; --m_Iter; return *temp; }

    ConstIterator& operator = (const Iterator& r) { m_Pos = r.m_Pos; m_Iter = r.m_Iter; return *this; }
    
    bool operator == (const Iterator& r) const { return m_Iter == r.m_Iter; }
    bool operator != (const Iterator& r) const { return m_Iter != r.m_Iter; }
    bool operator == (const ConstIterator& r) const { return m_Iter == r.m_Iter; }
    bool operator != (const ConstIterator& r) const { return m_Iter != r.m_Iter; }
    
    /** Get the index into the SmartPointerVectorContainer associated with this iterator.   */
    ElementIdentifier Index(void) const { return static_cast<ElementIdentifier>( m_Pos ); }
    
    /** Get the value at this iterator's location in the SmartPointerVectorContainer.   */
    const Element* Value(void) const { return m_Iter->GetPointer(); }
    
  private:
    size_type m_Pos;
    const VectorType* m_Vector;
    VectorConstIterator m_Iter;
    friend class Iterator;
  };  
  
  /** Declare the public interface routines. */
  ElementPointer& ElementAt(ElementIdentifier);
  ConstElementPointer ElementAt(ElementIdentifier) const; // no need to return reference, because assignment not allowed!
  ElementPointer& CreateElementAt(ElementIdentifier);
  ConstElementPointer GetElement(ElementIdentifier) const;
  void SetElement(ElementIdentifier, Element*);
  void InsertElement(ElementIdentifier, Element*);
  bool IndexExists(ElementIdentifier) const;
  bool GetElementIfIndexExists(ElementIdentifier, Element**) const;
  void CreateIndex(ElementIdentifier);
  void DeleteIndex(ElementIdentifier);
  ConstIterator Begin(void) const;
  ConstIterator End(void) const;  
  Iterator Begin(void);
  Iterator End(void);  
  unsigned long Size(void) const;
  void Reserve(ElementIdentifier);
  void Squeeze(void);
  void Initialize(void);
    
};

} // end namespace itk
  
#ifndef ITK_MANUAL_INSTANTIATION
#include "itkSmartPointerVectorContainer.txx"
#endif

#endif


