/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef USATOMICINT_H
#define USATOMICINT_H

#include <usGlobalConfig.h>
#include <MitkCppMicroServicesExports.h>
#include <usThreads_p.h>

namespace us {

/**
 * This class acts as an atomic integer.
 *
 * The integer value represented by this class can be incremented
 * and decremented atomically. This is often useful in reference
 * counting scenarios to minimize locking overhead in multi-threaded
 * environments.
 */
class AtomicInt : private AtomicCounter
{

public:

  AtomicInt(int value = 0) : AtomicCounter(value) {}

  /**
   * Increase the reference count atomically by 1.
   *
   * \return <code>true</code> if the new value is unequal to zero, <code>false</code>
   *         otherwise.
   */
  inline bool Ref() const
  { return AtomicIncrement() != 0; }

  /**
   * Decrease the reference count atomically by 1.
   *
   * \return <code>true</code> if the new value is unequal to zero, <code>false</code>
   *         otherwise.
   */
  inline bool Deref() const
  { return AtomicDecrement() != 0; }

  /**
   * Returns the current value.
   *
   */
  inline operator int() const
  {
    IntType curr(0);
    AtomicAssign(curr);
    return curr;
  }

};

}

#endif // USATOMICINT_H
