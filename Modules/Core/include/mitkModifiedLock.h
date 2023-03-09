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
  //##Documentation
  //## @brief ModifiedLock manages the calls of Modified() functions
  //##
  //## If an object of ModifiedLock is created, the ModifiedLockFlag in class
  //## BaseGeometry is set to true. Therefore, all following calls of Modified()
  //## will be collected and only be carried out at the end of the function / section,
  //## when the deconstructor of the ModifiedLock object is called.
  //##
  class MITKCORE_EXPORT ModifiedLock
  {
  public:
    ModifiedLock();
    ModifiedLock(BaseGeometry *baseGeo);
    ~ModifiedLock();

  private:
    BaseGeometry *m_baseGeometry;
  };
}

#endif
