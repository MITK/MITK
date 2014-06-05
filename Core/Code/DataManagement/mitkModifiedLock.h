/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/
#ifndef ModifiedLock_H_HEADER_INCLUDED
#define ModifiedLock_H_HEADER_INCLUDED

#include <MitkCoreExports.h>
#include <mitkBaseGeometry.h>
namespace mitk {
  //##Documentation
  //## @brief ModifiedLock manages the calls of Modified() functions
  //##
  //## If an object of ModifiedLock is created, the ModifiedLockFlag in class
  //## BaseGeometry is set to true. Therefore, all following calls of Modified()
  //## will be collected and only be carried out at the end of the function / section,
  //## when the deconstructor of the ModifiedLock object is called.
  //##
  class MITK_CORE_EXPORT ModifiedLock
  {
  public:
    ModifiedLock();
    ModifiedLock(BaseGeometry* baseGeo);
    ~ModifiedLock();
  private:
    BaseGeometry* m_baseGeometry;
  };
}

#endif //Header
