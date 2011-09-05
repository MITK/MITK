/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
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

#include "mitkAtomicInt.h"

#if defined(__APPLE__)
  // OSAtomic.h optimizations only used in 10.5 and later
  #if MAC_OS_X_VERSION_MAX_ALLOWED >= 1050
    #include <libkern/OSAtomic.h>
  #endif

#elif defined(__GLIBCPP__) || defined(__GLIBCXX__)
  #if (__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 2))
  # include <ext/atomicity.h>
  #else
  # include <bits/atomicity.h>
  #endif

#endif


namespace mitk
{

AtomicInt::AtomicInt(int value)
  : m_ReferenceCount(value)
{

}

bool AtomicInt::Ref() const
{
  // Windows optimization
#if (defined(WIN32) || defined(_WIN32))
  return InterlockedIncrement(&m_ReferenceCount);

  // Mac optimization
#elif defined(__APPLE__) && (MAC_OS_X_VERSION_MIN_REQUIRED >= 1050)
 #if defined (__LP64__) && __LP64__
  return OSAtomicIncrement64Barrier(&m_ReferenceCount);
 #else
  return OSAtomicIncrement32Barrier(&m_ReferenceCount);
 #endif

  // gcc optimization
#elif defined(__GLIBCPP__) || defined(__GLIBCXX__)
  return __sync_add_and_fetch(&m_ReferenceCount, 1);

  // General case
#else
  m_ReferenceCountLock.Lock();
  InternalReferenceCountType ref = ++m_ReferenceCount;
  m_ReferenceCountLock.Unlock();
  return ref;
#endif
}

bool AtomicInt::Deref() const
{
  // Windows optimization
#if (defined(WIN32) || defined(_WIN32))
  return InterlockedDecrement(&m_ReferenceCount);

// Mac optimization
#elif defined(__APPLE__) && (MAC_OS_X_VERSION_MIN_REQUIRED >= 1050)
 #if defined (__LP64__) && __LP64__
  return OSAtomicDecrement64Barrier(&m_ReferenceCount);
 #else
  return OSAtomicDecrement32Barrier(&m_ReferenceCount);
 #endif

// gcc optimization
#elif defined(__GLIBCPP__) || defined(__GLIBCXX__)
  return __sync_add_and_fetch(&m_ReferenceCount, -1);

// General case
#else
  m_ReferenceCountLock.Lock();
  InternalReferenceCountType ref = --m_ReferenceCount;
  m_ReferenceCountLock.Unlock();
  return ref;
#endif
}


} // end namespace itk
