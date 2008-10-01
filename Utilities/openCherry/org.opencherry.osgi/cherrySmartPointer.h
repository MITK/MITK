/*=========================================================================

Program:   openCherry Platform
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef CHERRYOSGISMARTPOINTER_H_
#define CHERRYOSGISMARTPOINTER_H_

#include <iostream>

#include "cherryOSGiDll.h"

namespace cherry {

/** \class SmartPointer
 * \brief Implements transparent reference counting.
 *
 * SmartPointer is a copy of itk::SmartPointer.
 * It implements reference counting by overloading
 * operator -> (and *) among others. This allows natural interface
 * to the class referred to by the pointer without having to invoke
 * special Register()/UnRegister() methods directly.
 *
 */
template <class TObjectType>
class SmartPointer
{
public:
  typedef TObjectType ObjectType;

  /** Constructor  */
  SmartPointer ()
    { m_Pointer = 0; }

  /** Copy constructor  */
  SmartPointer (const SmartPointer<ObjectType> &p):
    m_Pointer(p.m_Pointer)
    { this->Register(); }

  template <class Other>
  SmartPointer (const SmartPointer<Other>& ptr):
    m_Pointer(const_cast<Other*>(ptr.GetPointer()))
    { if (m_Pointer) this->Register(); }

  /** Constructor to pointer p  */
  SmartPointer (ObjectType *p):
    m_Pointer(p)
    { this->Register(); }

  /** Destructor  */
  ~SmartPointer ()
    {
    this->UnRegister();
    m_Pointer = 0;
    }

  template <class Other>
  SmartPointer<Other> Cast() const
    {
      Other* pOther = dynamic_cast<Other*>(m_Pointer);
      return SmartPointer<Other>(pOther);
    }

  /** Overload operator ->  */
  ObjectType *operator -> () const
    { return m_Pointer; }

  /** Return pointer to object.  */
  operator ObjectType * () const
    { return m_Pointer; }

  /** Test if the pointer has been initialized */
  bool IsNotNull() const
  { return m_Pointer != 0; }
  bool IsNull() const
  { return m_Pointer == 0; }

  /** Template comparison operators. */
  template <typename R>
  bool operator == ( R r ) const
    { return (m_Pointer == static_cast<const ObjectType*>(r) ); }

  template <typename R>
  bool operator == ( SmartPointer<R> r ) const
    { return (m_Pointer == static_cast<const ObjectType*>(r.GetPointer()) ); }

  bool operator == ( int r ) const
    { return (r == 0 ? m_Pointer == 0 : false ); }

  template <typename R>
  bool operator != ( R r ) const
    { return (m_Pointer != static_cast<const ObjectType*>(r) ); }

  template <typename R >
  bool operator != ( SmartPointer<R> r ) const
    { return (m_Pointer != static_cast<const ObjectType*>(r.GetPointer()) ); }

  bool operator != ( int r ) const
    { return (r == 0 ? m_Pointer != 0 : false ); }

  /** Template comparison operators using operator==. */
  template <typename R>
  bool CompareTo ( R r ) const
    { const ObjectType* o = static_cast<const ObjectType*>(r);
      return m_Pointer == 0 ? o == 0 : (o && m_Pointer->operator==(o) ); }

  /** Access function to pointer. */
  ObjectType *GetPointer () const
    { return m_Pointer; }

  /** Comparison of pointers. Less than comparison.  */
  bool operator < (const SmartPointer &r) const
    { return (void*)m_Pointer < (void*) r.m_Pointer; }

  /** Comparison of pointers. Greater than comparison.  */
  bool operator > (const SmartPointer &r) const
    { return (void*)m_Pointer > (void*) r.m_Pointer; }

  /** Comparison of pointers. Less than or equal to comparison.  */
  bool operator <= (const SmartPointer &r) const
    { return (void*)m_Pointer <= (void*) r.m_Pointer; }

  /** Comparison of pointers. Greater than or equal to comparison.  */
  bool operator >= (const SmartPointer &r) const
    { return (void*)m_Pointer >= (void*) r.m_Pointer; }

  /** Overload operator assignment.  */
  SmartPointer &operator = (const SmartPointer &r)
    { return this->operator = (r.GetPointer()); }

  /** Overload operator assignment.  */
  SmartPointer &operator = (ObjectType *r)
    {
    if (m_Pointer != r)
      {
      ObjectType* tmp = m_Pointer; //avoid recursive unregisters by retaining temporarily
      m_Pointer = r;
      this->Register();
      if ( tmp ) { tmp->UnRegister(); }
      }
    return *this;
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

  void Register()
    {
    if(m_Pointer) { m_Pointer->Register(); }
    }

  void UnRegister()
    {
    if(m_Pointer) { m_Pointer->UnRegister(); }
    }
};


template <typename T>
std::ostream& operator<< (std::ostream& os, SmartPointer<T> p)
{
  p.Print(os);
  return os;
}

}

#endif /*CHERRYOSGISMARTPOINTER_H_*/
