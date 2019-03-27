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

#ifndef BERRYOBJECT_H_
#define BERRYOBJECT_H_

#include <org_blueberry_core_runtime_Export.h>

#include "berryMacros.h"
#include "berryMessage.h"

#include <berryConfig.h>

#include <QString>
#include <QMetaType>
#include <QMutex>
#include <QAtomicInt>

#ifdef _MSC_VER
// disable inheritance by dominance warnings
#pragma warning( disable : 4250 4275 )
#endif

class QDebug;
class QTextStream;

namespace berry {

class org_blueberry_core_runtime_EXPORT Indent
{

public:

  /** Standard class typedefs. */
  typedef Indent  Self;

  /** Construct the object with an initial Indentation level. */
  Indent(int ind = 0) { m_Indent=ind; }

  /**
   * Determine the next Indentation level. Keep Indenting by two until the
   * a maximum of forty spaces is reached.
   */
  Indent GetNextIndent();

  /** Print out the Indentation. Basically output a bunch of spaces.  */
  friend org_blueberry_core_runtime_EXPORT QDebug operator<<(QDebug os, const Indent& o);

private:

  int m_Indent;
};


/** \class Object
 * \brief Light weight base class for most BlueBerry classes.
 *
 * Object is copied from itk::LightObject and is the highest
 * level base class for most BlueBerry objects. It
 * implements reference counting and the API for object printing.
 *
 */
class org_blueberry_core_runtime_EXPORT Object
{

private:

  mutable Message<> m_DestroyMessage;

public:

  typedef Object Self;
  typedef berry::SmartPointer<Self> Pointer;
  typedef berry::SmartPointer<const Self>  ConstPointer;
  typedef berry::WeakPointer<Self> WeakPtr;
  typedef berry::WeakPointer<const Self> ConstWeakPtr;

  static const char* GetStaticClassName();
  virtual QString GetClassName() const;

  static Reflection::TypeInfo GetStaticTypeInfo();
  virtual Reflection::TypeInfo GetTypeInfo() const;

  static QList<Reflection::TypeInfo> GetStaticSuperclasses();
  virtual QList<Reflection::TypeInfo> GetSuperclasses() const;

  /** Delete an BlueBerry object. This method should always be used to delete an
   * object when the new operator was used to create it. Using the C
   * delete method will not work with reference counting.  */
  virtual void Delete();


#ifdef _WIN32
  /** Used to avoid dll boundary problems.  */
  void* operator new(size_t);
  void* operator new[](size_t);
  void operator delete(void*);
  void operator delete[](void*, size_t);
#endif

  /**
   * Cause the object to print itself out. This is usually used to provide
   * detailed information about the object's state. It just calls the
   * header/self/trailer virtual print methods, which can be overriden by
   * subclasses.
   */
  QDebug Print(QDebug os, Indent Indent=0) const;

  /**
   * Returns a string representation of this object. The default
   * implementation returns an empty string.
   */
  virtual QString ToString() const;

  /**
   * Returns a hash code value for the object.
   */
  virtual uint HashCode() const;

  /**
   * Override this method to implement a specific "less than" operator
   * for associative STL containers.
   */
  virtual bool operator<(const Object*) const;

  /** Increase the reference count (mark as used by another object).  */
  void Register() const;

  /** Decrease the reference count (release by another object).
   * Set del to false if you do not want the object to be deleted if
   * the reference count is zero (use with care!) */
  void UnRegister(bool del = true) const;

  /** Gets the reference count on this object. */
  int GetReferenceCount() const
  {
    return m_ReferenceCount.load();
  }

  /** Sets the reference count on this object. This is a dangerous
   * method, use it with care. */
  void SetReferenceCount(int);

  inline void AddDestroyListener(const MessageAbstractDelegate<>& delegate) const
  { m_DestroyMessage += delegate; }
  inline void RemoveDestroyListener(const MessageAbstractDelegate<>& delegate) const
  { m_DestroyMessage -= delegate; }


  /**
   * A generic comparison method. Override this method in subclasses and
   * cast to your derived class to provide a more detailed comparison.
   */
  virtual bool operator==(const Object*) const;

#ifdef BLUEBERRY_DEBUG_SMARTPOINTER
  unsigned int GetTraceId() const;
private:
  unsigned int m_TraceId;
  unsigned int& GetTraceIdCounter() const;
public:
#endif

protected:

  friend struct QScopedPointerObjectDeleter;

  Object();
  virtual ~Object();

  /**
   * Methods invoked by Print() to print information about the object
   * including superclasses. Typically not called by the user (use Print()
   * instead) but used in the hierarchical print process to combine the
   * output of several classes.
   */
  virtual QDebug PrintSelf(QDebug os, Indent indent) const;
  virtual QDebug PrintHeader(QDebug os, Indent indent) const;
  virtual QDebug PrintTrailer(QDebug os, Indent indent) const;

  /** Number of uses of this object by other objects. */
  mutable QAtomicInt m_ReferenceCount;

  /** Mutex lock to protect modification to the reference count */
  mutable QMutex m_ReferenceCountLock;

private:

  Object(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented

};

// A custom deleter for QScopedPointer
// berry::Object instances in a QScopedPointer should have reference count one,
// such that they are not accidentally deleted when a temporary smart pointer
// pointing to it goes out of scope. This deleter fixes the reference count and
// always deletes the instance. Use a berry::SmartPointer if the lifetime should
// exceed the one of the pointer.
struct QScopedPointerObjectDeleter
{
  static inline void cleanup(Object* obj)
  {
    if (obj == nullptr) return;
    obj->UnRegister(false);
    delete obj;
  }
};

org_blueberry_core_runtime_EXPORT QDebug operator<<(QDebug os, const berry::Indent& o);

}

/**
 * This operator allows all subclasses of Object to be printed via QDebug <<.
 * It in turn invokes the Print method, which in turn will invoke the
 * PrintSelf method that all objects should define, if they have anything
 * interesting to print out.
 */
org_blueberry_core_runtime_EXPORT QDebug operator<<(QDebug os, const berry::Object& o);
org_blueberry_core_runtime_EXPORT QDebug operator<<(QDebug os, const berry::SmartPointer<const berry::Object>& o);
org_blueberry_core_runtime_EXPORT QDebug operator<<(QDebug os, const berry::SmartPointer<berry::Object>& o);

org_blueberry_core_runtime_EXPORT QTextStream& operator<<(QTextStream& os, const berry::Object& o);
org_blueberry_core_runtime_EXPORT QTextStream& operator<<(QTextStream& os, const berry::SmartPointer<const berry::Object>& o);
//org_blueberry_core_runtime_EXPORT QTextStream& operator<<(QTextStream& os, const berry::SmartPointer<berry::Object>& o);

Q_DECLARE_METATYPE(berry::Object::Pointer)

org_blueberry_core_runtime_EXPORT uint qHash(const berry::Object& o);

#endif /*BERRYOBJECT_H_*/
