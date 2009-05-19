/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: 14123 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef __mitkWeakPointer_h
#define __mitkWeakPointer_h

#include "mitkCommon.h"
#include "mitkMessage.h"

#include <itkCommand.h>
#include <iostream>

namespace mitk
{

/** \class WeakPointer
 * \brief Implements a weak reference to an object.
 *
 * Extends the standard itk WeakPointer by listening to delete events of itk::Objects.
 * When an itk::Object is deleted the WeakPointer sets its internal Pointer to 0.
 * This enables checking against 0 and avoids crashes by accessing changed memory.
 * Furthermore it dispatches Modified events with the mitkMessageDelegate system which is
 * much easier to use.
 */
template <class TObjectType>
class WeakPointer
{
public:
  /** Extract infoirmation from template parameter. */
  typedef TObjectType ObjectType;

  typedef Message1<const itk::Object*> itkObjectEvent;
  //##Documentation
  //## @brief AddEvent is emitted when the object pointed to gets deleted
  itkObjectEvent ObjectDelete;

  //##Documentation
  //## @brief AddEvent is emitted when the object pointed to gets modified
  itkObjectEvent ObjectModified;

  /** Constructor.  */
  WeakPointer ()
    : m_DeleteObserverTag(-1)
    , m_ModifiedObserverTag(-1)
    , m_Pointer(0)
  {
  }

  /** Copy constructor.  */
  WeakPointer (const WeakPointer<ObjectType> &p)
    : m_DeleteObserverTag(-1)
    , m_ModifiedObserverTag(-1)
    , m_Pointer(p.m_Pointer)
  {
    this->AddDeleteAndModifiedObserver();
  }

  /** Constructor to pointer p.  */
  WeakPointer (ObjectType *p)
    : m_DeleteObserverTag(0),
      m_Pointer(p) 
  {
    this->AddDeleteAndModifiedObserver();
  }

  /** Destructor.  */
  ~WeakPointer ()
  { 
    this->RemoveDeleteAndModifiedObserver();

    m_Pointer = 0; 
  }

  /** Overload operator ->.  */
  ObjectType *operator -> () const
    { return m_Pointer; }

  /** Return pointer to object.  */
  operator ObjectType * () const
    { return m_Pointer; }

  /** Template comparison operators. */
  template <typename R>
  bool operator == (R r) const
    {
    return (m_Pointer == (ObjectType*)r);
    }
  template <typename R>
  bool operator != (R r) const
    {
    return (m_Pointer != (ObjectType*)r);
    }

  /** Access function to pointer. */
  ObjectType *GetPointer () const
    { return m_Pointer; }

  /** Comparison of pointers. Less than comparison.  */
  bool operator < (const WeakPointer &r) const
    { return (void*)m_Pointer < (void*) r.m_Pointer; }

  /** Comparison of pointers. Greater than comparison.  */
  bool operator > (const WeakPointer &r) const
    { return (void*)m_Pointer > (void*) r.m_Pointer; }

  /** Comparison of pointers. Less than or equal to comparison.  */
  bool operator <= (const WeakPointer &r) const
    { return (void*)m_Pointer <= (void*) r.m_Pointer; }

  /** Comparison of pointers. Greater than or equal to comparison.  */
  bool operator >= (const WeakPointer &r) const
    { return (void*)m_Pointer >= (void*) r.m_Pointer; }

  /** Test if the pointer has been initialized */
  bool IsNotNull() const
  { return m_Pointer != 0; }
  bool IsNull() const
  { return m_Pointer == 0; }

  /** Overload operator assignment.  */
  WeakPointer &operator = (const WeakPointer &r)
    { return this->operator = (r.GetPointer()); }

  /** Overload operator assignment.  */
  WeakPointer &operator = (ObjectType *r)
  {
    this->RemoveDeleteAndModifiedObserver();
    m_Pointer = r;
    this->AddDeleteAndModifiedObserver();
    return *this;
  }

  /** Function to print object pointed to.  */
  ObjectType *Print (std::ostream& os) const
  {
  // This prints the object pointed to by the pointer
  (*m_Pointer).Print(os);
  return m_Pointer;
  }

  ///
  /// \brief Gets called when the object is deleted or modified.
  ///
  void OnObjectDelete( const itk::Object *caller, const itk::EventObject &event )
  {
    this->RemoveDeleteAndModifiedObserver();
    m_Pointer = 0;
    ObjectDelete.Send(caller);

  }

  void OnObjectModified( const itk::Object *caller, const itk::EventObject &event )
  {
    ObjectModified.Send(caller);
  }

private:
  void AddDeleteAndModifiedObserver()
  {
    if(m_DeleteObserverTag == -1 && m_ModifiedObserverTag == -1 && m_Pointer != 0)
    {
      // add observer for delete event
      typename itk::MemberCommand<WeakPointer<TObjectType> >::Pointer onObjectDelete =
        itk::MemberCommand<WeakPointer<TObjectType> >::New();

      onObjectDelete->SetCallbackFunction(this, &WeakPointer<TObjectType>::OnObjectDelete);
      m_DeleteObserverTag = m_Pointer->AddObserver(itk::DeleteEvent(), onObjectDelete);

      // add observer for modified event
      typename itk::MemberCommand<WeakPointer<TObjectType> >::Pointer onObjectModified =
        itk::MemberCommand<WeakPointer<TObjectType> >::New();

      onObjectModified->SetCallbackFunction(this, &WeakPointer<TObjectType>::OnObjectModified);
      m_ModifiedObserverTag = m_Pointer->AddObserver(itk::ModifiedEvent(), onObjectModified);
    }
  }

  void RemoveDeleteAndModifiedObserver()
  {
    if(m_DeleteObserverTag >= 0 && m_ModifiedObserverTag >= 0 && m_Pointer != 0)
    {      
      m_Pointer->RemoveObserver(m_DeleteObserverTag);
      m_Pointer->RemoveObserver(m_ModifiedObserverTag);

      m_DeleteObserverTag = -1;
      m_ModifiedObserverTag = -1;
    }
  }

  long m_DeleteObserverTag;
  long m_ModifiedObserverTag;

  /** The pointer to the object referrred to by this smart pointer. */
  ObjectType* m_Pointer;
};


template <typename T>
std::ostream& operator<< (std::ostream& os, WeakPointer<T> p)
{
  p.Print(os);
  return os;
}

} // end namespace mitk

#endif
