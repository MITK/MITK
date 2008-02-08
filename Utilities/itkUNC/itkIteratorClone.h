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
#ifndef __itkIteratorClone_h
#define __itkIteratorClone_h

#include "itkMacro.h"
#include <iostream>

namespace itk
{

template <class TObjectType>
class IteratorClone 
{
public:
  typedef IteratorClone<TObjectType> Self;
  typedef TObjectType ObjectType;
  
  /** Constructor  */
  IteratorClone () 
    { m_Pointer = 0; }

  /** Copy constructor  */
  IteratorClone (const IteratorClone<ObjectType> &p):
    m_Pointer(0)
    { 
    if(p.m_Pointer!=NULL)
      m_Pointer=p.m_Pointer->Clone();
    }
  
  /** Constructor to pointer p  */
  IteratorClone (ObjectType *p):
    m_Pointer(0)
    { 
    if(p!=NULL) 
      m_Pointer=p->Clone();
    }                             
  
  /** Constructor to reference p  */
  IteratorClone (const ObjectType &p):
    m_Pointer(0)
    { 
    m_Pointer=const_cast<ObjectType*>(&p)->Clone();
    }                             
  
  /** Destructor  */
  ~IteratorClone ()
    {
    delete m_Pointer;
    m_Pointer = 0;
    }
  
  /** Overload operator ->  */
  ObjectType *operator -> () const
    { return m_Pointer; }

  /** Test if the pointer has been initialized */
  bool IsNotNull() const
  { return m_Pointer != 0; }
  bool IsNull() const
  { return m_Pointer == 0; }

  /** Template comparison operators. */
  template <typename R>
  bool operator == ( R r ) const
    { return (m_Pointer == (ObjectType*)(r) ); }

  template <typename R>
  bool operator != ( R r ) const
    { return (m_Pointer != (ObjectType*)(r) ); }
    
  /** Access function to pointer. */
  ObjectType *GetPointer () const 
    { return m_Pointer; }
  
  /** Comparison of pointers. Less than comparison.  */
  bool operator < (const IteratorClone &r) const
    { return (void*)m_Pointer < (void*) r.m_Pointer; }
  
  /** Comparison of pointers. Greater than comparison.  */
  bool operator > (const IteratorClone &r) const
    { return (void*)m_Pointer > (void*) r.m_Pointer; }

  /** Comparison of pointers. Less than or equal to comparison.  */
  bool operator <= (const IteratorClone &r) const
    { return (void*)m_Pointer <= (void*) r.m_Pointer; }

  /** Comparison of pointers. Greater than or equal to comparison.  */
  bool operator >= (const IteratorClone &r) const
    { return (void*)m_Pointer >= (void*) r.m_Pointer; }

  /** Overload operator assignment.  */
  IteratorClone &operator = (const IteratorClone &r)
    { return this->operator = (r.GetPointer()); }
  
  /** Overload operator assignment.  */
  IteratorClone &operator = (const ObjectType *r)
    {                                                              
    if (m_Pointer != r)
      {
      delete m_Pointer;
      m_Pointer = 0;  
      if(r!=NULL) 
        m_Pointer=const_cast<ObjectType*>(r)->Clone();
      }
    return *this;
    }

  Self &
  operator++()
    {
    if(m_Pointer)
      ++(*m_Pointer);
    return *this;
    }

  const Self
  operator++(int)
    {
    if(m_Pointer)
      {
      const Self oldValue( m_Pointer ); // create a copy of the iterator behind the pointer (Clone())
      ++(*m_Pointer);
      return oldValue;
      }
    }
   
  /** Function to print object pointed to  */
  ObjectType *Print (std::ostream& os) const 
    { 
      // This prints the object pointed to by the pointer  
      (*m_Pointer).Print(os);  
      return m_Pointer;
    } 

private:
  /** The pointer to the object referrred to by this smart pointer. */
  ObjectType* m_Pointer;
};  

  
template <typename T>
std::ostream& operator<< (std::ostream& os, IteratorClone<T> p) 
{
  p.Print(os); 
  return os;
}

} // end namespace itk
  
#endif
