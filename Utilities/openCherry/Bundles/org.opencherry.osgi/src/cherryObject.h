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

#ifndef CHERRYOSGILIGHTOBJECT_H_
#define CHERRYOSGILIGHTOBJECT_H_

#include <Poco/Mutex.h>

#include "cherryOSGiDll.h"
#include "cherryMacros.h"
#include "cherryMessage.h"

namespace cherry {


class CHERRY_OSGI Indent
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
  static const char *GetNameOfClass() {return "Indent";}

  /** Determine the next Indentation level. Keep Indenting by two until the
   * a maximum of forty spaces is reached.  */
  Indent GetNextIndent();

  /** Print out the Indentation. Basically output a bunch of spaces.  */
  friend CHERRY_OSGI std::ostream& operator<<(std::ostream& os, const Indent& o);

private:
  int m_Indent;
};

//CHERRY_OSGI std::ostream& operator<<(std::ostream& os, const Indent& o);


/** \class Object
 * \brief Light weight base class for most cherry OSGi classes.
 *
 * Object is copied from itk::LightObject and is the highest
 * level base class for most cherry osgi objects. It
 * implements reference counting and the API for object printing.
 *
 */
class CHERRY_OSGI Object
{

 /**
 *  in this case the message macro is used to create a message object as well as add / remove functions
 *  (AddDestroyListener, Removedestroylistener) on the message object
 *  in order to call the registered functions when the object is destroyed.
 *  More information about the NewMessageMacro can be found in @see cherryMessage.h
 *
 */
 cherryNewMessageMacro(Destroy)

public:

  cherryObjectMacro(Object)

  /** Delete an itk object.  This method should always be used to delete an
   * object when the new operator was used to create it. Using the C
   *  delete method will not work with reference counting.  */
  virtual void Delete();


  /** Returns a unique name which can be used to name
     * library manifests for dynamic class loading. Used in the
     * extension point mechanism.
     *
     * This method will be declared by using the cherryInterfaceMacro or
     * cherryManifestMacro macros.
     */
//  static const char *GetManifestName() {
//    return "cherry::Object";
//  }

#ifdef _WIN32
  /** Used to avoid dll boundary problems.  */
  void* operator new(size_t);
  void* operator new[](size_t);
  void operator delete(void*);
  void operator delete[](void*, size_t);
#endif

  /** Cause the object to print itself out. */
  void Print(std::ostream& os, Indent Indent=0) const;

  /** This method is called when itkExceptionMacro executes. It allows
   * the debugger to break on error.  */
  static void BreakOnError();

  /** Increase the reference count (mark as used by another object).  */
  virtual void Register() const;

  /** Decrease the reference count (release by another object).  */
  virtual void UnRegister() const;

  /** Gets the reference count on this object. */
  virtual int GetReferenceCount() const
    {return m_ReferenceCount;}

  /** Sets the reference count on this object. This is a dangerous
   * method, use it with care. */
  virtual void SetReferenceCount(int);

  virtual bool operator==(const Object*) const;

#ifdef CHERRY_DEBUG_SMARTPOINTER
  unsigned long GetTraceId() const;
private:
  unsigned long m_TraceId;
  unsigned long& GetTraceIdCounter() const;
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


}

#endif /*CHERRYOSGILIGHTOBJECT_H_*/
