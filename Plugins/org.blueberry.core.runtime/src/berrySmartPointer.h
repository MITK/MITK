/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef BERRYSMARTPOINTER_H_
#define BERRYSMARTPOINTER_H_

#include <iostream>
#include <stdexcept>

#include <org_blueberry_core_runtime_Export.h>

#include <berryConfig.h>

#include <Poco/Bugcheck.h>

#if defined(BLUEBERRY_DEBUG_SMARTPOINTER)
#include <QMutex>
#endif

namespace berry
{

template<class T> class WeakPointer;

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
template<class TObjectType>
class SmartPointer
{
public:
  typedef TObjectType ObjectType;
  typedef SmartPointer Self;

  /** Constructor  */
  SmartPointer() :
    m_Pointer(nullptr)
  {
#if defined(BLUEBERRY_DEBUG_SMARTPOINTER)
    DebugInitSmartPointer();
#endif

  }

  /** Constructor to pointer p  */
  explicit SmartPointer(ObjectType *p) :
    m_Pointer(p)
  {
    if (m_Pointer)
      this->Register();

#if defined(BLUEBERRY_DEBUG_SMARTPOINTER)
    DebugInitSmartPointer();
#endif
  }

  /** Copy constructor  */
  SmartPointer(const SmartPointer<ObjectType> &p) :
    m_Pointer(p.m_Pointer)
  {
    this->Register();

#if defined(BLUEBERRY_DEBUG_SMARTPOINTER)
    DebugInitSmartPointer();
#endif
  }

  template<class Other>
  SmartPointer(const SmartPointer<Other>& ptr) :
    m_Pointer(const_cast<Other*> (ptr.GetPointer()))
  {
    if (m_Pointer)
      this->Register();

#if defined(BLUEBERRY_DEBUG_SMARTPOINTER)
    DebugInitSmartPointer();
#endif
  }

  template<class Other>
  explicit SmartPointer(const WeakPointer<Other>& wp);

  /** Destructor  */
  ~SmartPointer()
  {
#if defined(BLUEBERRY_DEBUG_SMARTPOINTER)
    if (m_Pointer) DebugRemoveSmartPointer();
#endif

    this->UnRegister();
    m_Pointer = nullptr;
  }

  template<class Other>
  SmartPointer<Other> Cast() const
  {
    Other* pOther = dynamic_cast<Other*> (m_Pointer);
    return SmartPointer<Other> (pOther);
  }

  /** Overload operator ->  */
  ObjectType *operator ->() const
  {
    return m_Pointer;
  }

//  /** Return pointer to object.  */
//  operator ObjectType *() const
//  {
//    return m_Pointer;
//  }

  ObjectType & operator*() const
  {
    poco_assert( m_Pointer != nullptr );
    return *m_Pointer;
  }

  /** Test if the pointer has been initialized */
  bool IsNotNull() const
  {
    return m_Pointer != nullptr;
  }
  bool IsNull() const
  {
    return m_Pointer == nullptr;
  }

  typedef ObjectType * Self::*unspecified_bool_type;

  operator unspecified_bool_type () const
  {
    return m_Pointer == nullptr ? nullptr: &Self::m_Pointer;
  }

  /** Template comparison operators. */
  template<typename R>
  bool operator ==(const R* o) const
  {
    return (m_Pointer == nullptr ? o == nullptr : (o && m_Pointer->operator==(o)));
  }

  template<typename R>
  bool operator ==(const SmartPointer<R>& r) const
  {
    const R* o = r.GetPointer();
    return (m_Pointer == nullptr ? o == nullptr : (o && m_Pointer->operator==(o)));
  }

  bool operator ==(int r) const
  {
    if (r == 0)
      return m_Pointer == nullptr;

    throw std::invalid_argument("Can only compare to 0");
  }

  template<typename R>
  bool operator !=(const R* r) const
  {
    return !(this->operator==(r));
  }

  template<typename R>
  bool operator !=(const SmartPointer<R>& r) const
  {
    return !(this->operator==(r));
  }

  bool operator !=(int r) const
  {
    if (r == 0)
      return m_Pointer != nullptr;

    throw std::invalid_argument("Can only compare to 0");
  }

//  /** Template comparison operators using operator==. */
//  template<typename R>
//  bool CompareTo(const SmartPointer<R>& r) const
//  {
//    return m_Pointer == 0 ? r == 0 : r.GetPointer() && m_Pointer->operator==(r.GetPointer());
//  }

//  template<typename R>
//  bool CompareTo(R r) const
//  {
//    //const ObjectType* o = static_cast<const ObjectType*> (r);
//    return m_Pointer == 0 ? r == 0 : (r && m_Pointer->operator==(r));
//  }

  /** Access function to pointer. */
  ObjectType *GetPointer() const
  {
    return m_Pointer;
  }

  /** Comparison of pointers. Less than comparison.  */
  template<typename R>
  bool operator <(const SmartPointer<R>& r) const
  {
    const R* o = r.GetPointer();
    return m_Pointer == nullptr ? o == nullptr : o && m_Pointer->operator<(o);
  }

  /** Comparison of pointers. Greater than comparison.  */
  template<typename R>
  bool operator>(const SmartPointer<R>& r) const
  {
    const R* o = r.GetPointer();
    return m_Pointer == 0 ? o == 0 : o && m_Pointer->operator>(o);
  }

  /** Comparison of pointers. Less than or equal to comparison.  */
  template<typename R>
  bool operator <=(const SmartPointer<R>& r) const
  {
    return this->operator<(r) || this->operator==(r);
  }

  /** Comparison of pointers. Greater than or equal to comparison.  */
  template<typename R>
  bool operator >=(const SmartPointer<R>& r) const
  {
    return this->operator>(r) || this->operator==(r);
  }

  /** Overload operator assignment.  */
  SmartPointer &operator =(const SmartPointer &r)
  {
    return this->operator =(r.GetPointer());
  }

  /** Overload operator assignment. */
  template<typename R>
  SmartPointer &operator =(const SmartPointer<R>& r)
  {
    return this->operator =(r.GetPointer());
  }

  /** Overload operator assignment.  */
  SmartPointer &operator =(ObjectType *r)
  {
    if (m_Pointer != r)
    {
#if defined(BLUEBERRY_DEBUG_SMARTPOINTER)
      DebugAssignSmartPointer(r, m_Pointer);
#endif
      ObjectType* tmp = m_Pointer; //avoid recursive unregisters by retaining temporarily
      m_Pointer = r;
      this->Register();
      if (tmp)
      {
        tmp->UnRegister();
      }
    }
    return *this;
  }

  /** Function to print object pointed to  */
  QDebug Print(QDebug os) const;

private:

  /** The pointer to the object referred to by this smart pointer. */
  ObjectType* m_Pointer;

  void Register()
  {
    if (m_Pointer)
    {
      m_Pointer->Register();
    }
  }

  void UnRegister()
  {
    if (m_Pointer)
    {
      m_Pointer->UnRegister();
    }
  }

#if defined(BLUEBERRY_DEBUG_SMARTPOINTER)

  unsigned int m_Id;
  QMutex m_Mutex;

  void DebugInitSmartPointer();

  void DebugRemoveSmartPointer();

  void DebugAssignSmartPointer(const ObjectType* newObject, const ObjectType* oldObject);

public:

  int GetId();

private:
#endif
};

template<typename T>
std::ostream& operator<<(std::ostream& os, const SmartPointer<T>& p)
{
  os << p->ToString().toStdString();
  return os;
}

} // namespace berry

template<class T>
uint qHash(const berry::SmartPointer<T>& sp)
{
  return sp->HashCode();
}

template<class T>
uint qHash(const berry::WeakPointer<T>& wp)
{
  berry::SmartPointer<T> sp(wp.Lock());
  if (sp.IsNull())
  {
    return 0;
  }
  return sp->HashCode();
}

#include "berryException.h"
#include <QDebug>

namespace berry {

template<class T>
template<class Other>
SmartPointer<T>::SmartPointer(const WeakPointer<Other>& wp)
{
  if (wp.m_Pointer)
  {
    this->m_Pointer = wp.m_Pointer;
    this->Register();

    #if defined(BLUEBERRY_DEBUG_SMARTPOINTER)
      DebugInitSmartPointer();
    #endif
  }
  else
  {
    throw BadWeakPointerException("Weak pointer is nullptr");
  }
}

/** Function to print object pointed to  */
template<class T>
QDebug SmartPointer<T>::Print(QDebug os) const
{
  // This prints the object pointed to by the pointer
  (*m_Pointer).Print(os);
  return os;
}

}

#if defined(BLUEBERRY_DEBUG_SMARTPOINTER)

#include "berryDebugUtil.h"

namespace berry {

template<class T>
void SmartPointer<T>::DebugInitSmartPointer()
{
  {
    QMutexLocker lock(&m_Mutex);
    if (m_Pointer)
    {
      unsigned int& counter = DebugUtil::GetSmartPointerCounter();
      m_Id = ++counter;
      DebugUtil::RegisterSmartPointer(m_Id, m_Pointer);
    }
    else m_Id = 0;
  }

  //if (DebugUtil::GetSmartPointerCounter() == Platform::GetConfiguration().getInt(Platform::DEBUG_ARG_SMARTPOINTER_ID))
  //throw 1;
}

template<class T>
void SmartPointer<T>::DebugRemoveSmartPointer()
{
  QMutexLocker lock(&m_Mutex);
  DebugUtil::UnregisterSmartPointer(m_Id, m_Pointer);
}

template<class T>
void SmartPointer<T>::DebugAssignSmartPointer(const ObjectType* newObject, const ObjectType* oldObject)
{
  QMutexLocker lock(&m_Mutex);
  if (oldObject)
    DebugUtil::UnregisterSmartPointer(m_Id, oldObject);

  if (newObject)
  {
    if (m_Id < 1)
    {
      unsigned int& counter = DebugUtil::GetSmartPointerCounter();
      m_Id = ++counter;
    }
    DebugUtil::RegisterSmartPointer(m_Id, newObject);
  }
}

template<class T>
int SmartPointer<T>::GetId()
{
  return m_Id;
}

}

#endif

#endif /*BERRYSMARTPOINTER_H_*/
