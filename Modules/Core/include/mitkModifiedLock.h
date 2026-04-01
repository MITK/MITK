/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef mitkModifiedLock_h
#define mitkModifiedLock_h

#include <MitkCoreExports.h>
#include <mitkBaseGeometry.h>
namespace mitk
{
  /**
   * \brief RAII guard that defers BaseGeometry::Modified() calls until destruction.
   *
   * When an object of ModifiedLock is created, the ModifiedLockFlag in the associated
   * BaseGeometry is set to true. Therefore, all subsequent calls to Modified()
   * are collected and only carried out when the destructor of this ModifiedLock
   * object is called, ensuring that Modified() is invoked at most once.
   */
  class MITKCORE_EXPORT ModifiedLock
  {
  public:
    /** \brief Default constructor. Does not associate with any geometry. */
    ModifiedLock();

    /**
     * \brief Construct a ModifiedLock that defers Modified() calls on the given geometry.
     *
     * \param[in] baseGeo the BaseGeometry whose Modified() calls should be deferred.
     * \pre baseGeo must not be nullptr.
     */
    ModifiedLock(BaseGeometry *baseGeo);

    /**
     * \brief Destructor. Releases the lock and calls Modified() on the geometry
     * if any Modified() calls were deferred during the lock's lifetime.
     */
    ~ModifiedLock();

  private:
    BaseGeometry *m_baseGeometry;
  };
}

#endif
