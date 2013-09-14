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

#ifndef BERRYOSGILIGHTOBJECT_H_
#define BERRYOSGILIGHTOBJECT_H_

#include <Poco/Mutex.h>
#include <string>

#include <org_blueberry_osgi_Export.h>
#include "berryMacros.h"
#include "berryMessage.h"

#include <berryConfig.h>

#ifdef _MSC_VER
// disable inheritance by dominance warnings
#pragma warning( disable : 4250 4275 4251 )
// disable warning: C++ exception specification ignored except to indicate a function is not __declspec(nothrow)
#pragma warning( disable : 4290 )
#endif

namespace berry {


class BERRY_OSGI Indent
{
public:
  /** Standard class typedefs. */
  typedef Indent  Self;

  /** Method for creation through the object factory. */
  static Self* New();

  /** Destroy this instance. */
  void Delete() {delete this;}

  /** Construct the object with an initial Indentation level. */
  Indent(int ind=0) {m_Indent=ind;}

  /** Return the name of the class. */
  static const char *GetClassName() {return "Indent";}

  /** Determine the next Indentation level. Keep Indenting by two until the
   * a maximum of forty spaces is reached.  */
  Indent GetNextIndent();

  /** Print out the Indentation. Basically output a bunch of spaces.  */
  friend BERRY_OSGI std::ostream& operator<<(std::ostream& os, const Indent& o);

private:
  int m_Indent;
};

BERRY_OSGI std::ostream& operator<<(std::ostream& os, const Indent& o);


/** \class Object
 * \brief Light weight base class for most BlueBerry classes.
 *
 * Object is copied from itk::LightObject and is the highest
 * level base class for most BlueBerry objects. It
 * implements reference counting and the API for object printing.
 *
 */
class BERRY_OSGI Object
{

 /**
 *  The message macro is used to create a message object as well as add / remove functions
 *  (AddDestroyListener, RemoveDestroyListener)
 *  in order to register functions which will be called when the object is destroyed.
 *  More information about the NewMessageMacro can be found in @see berryMessage.h
 *
 */
 berryNewMessageMacro(Destroy)

public:

  berryObjectMacro(Object);

  struct Hash {
    inline std::size_t operator()(const Self* value) const
    {
      return value->HashCode();
    }

    inline std::size_t operator()(Self::ConstPointer value) const
    {
      return value->HashCode();
    }

    inline std::size_t operator()(Self::WeakPtr value) const
    {
      try {
        const Self::ConstPointer sv(value.Lock());
        return sv->HashCode();
      }
      catch (BadWeakPointerException& /*e*/)
      {
        return 0;
      }
    }
  };

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

  /** Cause the object to print itself out. This is usually used to provide
   * detailed information about the object's state. It just calls the
 * header/self/trailer virtual print methods, which can be overriden by
 * subclasses. */
  void Print(std::ostream& os, Indent Indent=0) const;

  /** Returns a string representation of this object. */
  virtual std::string ToString() const;

  /** Returns a hash code value for the object. Use the Object::Hash functor
   * together with hashtable implementations.
   */
  virtual std::size_t HashCode() const;

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
    {return m_ReferenceCount;}

  /** Sets the reference count on this object. This is a dangerous
   * method, use it with care. */
  void SetReferenceCount(int);

  /** A generic comparison method. Override this method in subclasses and
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
  Object();
  virtual ~Object();

  /** Methods invoked by Print() to print information about the object
   * including superclasses. Typically not called by the user (use Print()
   * instead) but used in the hierarchical print process to combine the
   * output of several classes.  */
  virtual void PrintSelf(std::ostream& os, Indent indent) const;
  virtual void PrintHeader(std::ostream& os, Indent indent) const;
  virtual void PrintTrailer(std::ostream& os, Indent indent) const;

  /** Number of uses of this object by other objects. */
  mutable volatile int m_ReferenceCount;

  /** Mutex lock to protect modification to the reference count */
  mutable Poco::Mutex m_ReferenceCountLock;

private:
  Object(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented


};

/**
 * This operator allows all subclasses of Object to be printed via <<.
 * It in turn invokes the Print method, which in turn will invoke the
 * PrintSelf method that all objects should define, if they have anything
 * interesting to print out.
 */
BERRY_OSGI std::ostream& operator<<(std::ostream& os, const Object& o);


}

#endif /*BERRYOSGILIGHTOBJECT_H_*/
