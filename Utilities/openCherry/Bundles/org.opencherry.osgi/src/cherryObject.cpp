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

#include "cherryObject.h"

#include <list>
#include <memory>
#include <exception>

// Better name demangling for gcc
#if __GNUC__ > 3 || ( __GNUC__ == 3 && __GNUC_MINOR__ > 0 )
#define GCC_USEDEMANGLE
#endif

#ifdef GCC_USEDEMANGLE
#include <cstdlib>
#include <cxxabi.h>
#endif

namespace cherry
{


/**
 * Delete a itk object. This method should always be used to delete an object
 * when the new operator was used to create it. Using the C++ delete method
 * will not work with reference counting.
 */
void
Object
::Delete()
{
  this->UnRegister();
}


/**
 * Avoid DLL boundary problems.
 */
#ifdef _WIN32
void*
Object
::operator new(size_t n)
{
  return new char[n];
}

void*
Object
::operator new[](size_t n)
{
  return new char[n];
}

void
Object
::operator delete(void* m)
{
  delete [] (char*)m;
}

void
Object
::operator delete[](void* m, size_t)
{
  delete [] (char*)m;
}
#endif


/**
 * This function will be common to all itk objects.  It just calls the
 * header/self/trailer virtual print methods, which can be overriden by
 * subclasses (any itk object).
 */
void
Object
::Print(std::ostream& os, Indent indent) const
{
  this->PrintHeader(os, indent);
  this->PrintSelf(os, indent.GetNextIndent());
  this->PrintTrailer(os, indent);
}


/**
 * This method is called when itkExceptionMacro executes. It allows
 * the debugger to break on error.
 */
void
Object
::BreakOnError()
{
  ;
}


/**
 * Increase the reference count (mark as used by another object).
 */
void
Object
::Register() const
{
  Poco::Mutex::ScopedLock lock(m_ReferenceCountLock);
  m_ReferenceCount++;
}


/**
 * Decrease the reference count (release by another object).
 */
void
Object
::UnRegister() const
{
  m_ReferenceCountLock.lock();
  int tmpReferenceCount = --m_ReferenceCount;
  m_ReferenceCountLock.unlock();

  // ReferenceCount in now unlocked.  We may have a race condition
  // to delete the object.
  if ( tmpReferenceCount <= 0)
    {
    delete this;
    }
}


/**
 * Sets the reference count (use with care)
 */
void
Object
::SetReferenceCount(int ref)
{
  m_ReferenceCountLock.lock();
  m_ReferenceCount = ref;
  m_ReferenceCountLock.unlock();

  if ( m_ReferenceCount <= 0)
    {
    delete this;
    }
}

bool
Object::operator==(const Object* o) const
{
  return this == o;
}

#ifdef CHERRY_DEBUG_SMARTPOINTER
unsigned long Object::GetTraceId() const
{
  return m_TraceId;
}

unsigned long& Object::GetTraceIdCounter() const
{
  static unsigned long traceId = 0;
  return traceId;
}
#endif

Object::Object()
  :m_ReferenceCount(0)
{
#ifdef CHERRY_DEBUG_SMARTPOINTER
  unsigned long& id = GetTraceIdCounter();
  m_TraceId = ++id;
#endif
}

Object
::~Object()
{
  /**
   * warn user if reference counting is on and the object is being referenced
   * by another object.
   * a call to uncaught_exception is necessary here to avoid throwing an
   * exception if one has been thrown already. This is likely to
   * happen when a subclass constructor (say B) is throwing an exception: at
   * that point, the stack unwinds by calling all superclass destructors back
   * to this method (~Object): since the ref count is still 1, an
   * exception would be thrown again, causing the system to abort()!
   */
  if(m_ReferenceCount > 0 && !std::uncaught_exception())
    {
    // A general exception safety rule is that destructors should
    // never throw.  Something is wrong with a program that reaches
    // this point anyway.  Also this is the least-derived class so the
    // whole object has been destroyed by this point anyway.  Just
    // issue a warning.
    std::cout << "WARNING: In " __FILE__ ", line " << __LINE__ << "\n";
    std::cout << this->GetNameOfClass() << " (" << this << "): Trying to delete object with non-zero reference count.";
    std::cout << "\n\n";
    }
    /** 
    * calls the destroyListener added functions 
    */ 
    m_DestroyMessage.Send();
}


/**
 * Chaining method to print an object's instance variables, as well as
 * its superclasses.
 */
void
Object
::PrintSelf(std::ostream& os, Indent Indent) const
{
#ifdef GCC_USEDEMANGLE
  char const * mangledName = typeid(*this).name();
  int status;
  char* unmangled = abi::__cxa_demangle(mangledName, 0, 0, &status);

  os << Indent << "RTTI typeinfo:   ";

  if(status == 0)
    {
    os << unmangled;
    free(unmangled);
    }
  else
    {
    os << mangledName;
    }

  os << std::endl;
#else
  os << Indent << "RTTI typeinfo:   " << typeid( *this ).name() << std::endl;
#endif
  os << Indent << "Reference Count: " << m_ReferenceCount << std::endl;
}


/**
 * Define a default print header for all objects.
 */
void
Object
::PrintHeader(std::ostream& os, Indent Indent) const
{
  os << Indent << this->GetNameOfClass() << " (" << this << ")\n";
}


/**
 * Define a default print trailer for all objects.
 */
void
Object
::PrintTrailer(std::ostream& /*os*/, Indent /*Indent*/) const
{
}


/**
 * This operator allows all subclasses of Object to be printed via <<.
 * It in turn invokes the Print method, which in turn will invoke the
 * PrintSelf method that all objects should define, if they have anything
 * interesting to print out.
 */
std::ostream&
operator<<(std::ostream& os, const Object& o)
{
  o.Print(os);
  return os;
}


static const char blanks[41] =
"                                        ";

/**
 * Instance creation.
 */
Indent*
Indent::
New()
{
  return new Self;
}


/**
 * Determine the next Indentation level. Keep Indenting by two until the
 * max of forty.
 */
Indent
Indent
::GetNextIndent()
{
  int Indent = m_Indent + 2;
  if ( Indent > 40 )
    {
    Indent = 40;
    }
  return Indent;
}

/**
 * Print out the Indentation. Basically output a bunch of spaces.
 */
std::ostream&
operator<<(std::ostream& os, const Indent& ind)
{
  os << blanks + (40-ind.m_Indent) ;
  return os;
}


}
