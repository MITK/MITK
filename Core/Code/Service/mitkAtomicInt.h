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


#ifndef MITKATOMICINT_H
#define MITKATOMICINT_H

#include "itkFastMutexLock.h"

#include <MitkExports.h>

namespace mitk {

/**
 * This class acts as an atomic integer.
 *
 * The integer value represented by this class can be incremented
 * and decremented atomically. This is often useful in reference
 * counting scenarios to minimize locking overhead in multi-threaded
 * environments.
 */
class MITK_CORE_EXPORT AtomicInt {

public:

  AtomicInt(int value = 0);

  /**
   * Increase the reference count atomically by 1.
   *
   * \return <code>true</code> if the new value is unequal to zero, <code>false</code>
   *         otherwise.
   */
  bool Ref() const;

  /**
   * Decrease the reference count atomically by 1.
   *
   * \return <code>true</code> if the new value is unequal to zero, <code>false</code>
   *         otherwise.
   */
  bool Deref() const;

  // Non-atomic API

  /**
   * Returns the current value.
   *
   * This operator is not atomic.
   */
  inline operator int() const
  {
    return m_ReferenceCount;
  }

private:

  /** Define the type of the reference count according to the
      target. This allows the use of atomic operations */
#if (defined(WIN32) || defined(_WIN32))
  typedef LONG InternalReferenceCountType;
#elif defined(__APPLE__) && (MAC_OS_X_VERSION_MIN_REQUIRED >= 1050)
 #if defined (__LP64__) && __LP64__
  typedef volatile int64_t InternalReferenceCountType;
 #else
  typedef volatile int32_t InternalReferenceCountType;
 #endif
#elif defined(__GLIBCPP__) || defined(__GLIBCXX__)
  typedef _Atomic_word InternalReferenceCountType;
#else
  typedef int InternalReferenceCountType;
   mutable itk::SimpleFastMutexLock m_ReferenceCountLock;
#endif

  mutable InternalReferenceCountType m_ReferenceCount;
};

}

#endif // MITKATOMICINT_H
