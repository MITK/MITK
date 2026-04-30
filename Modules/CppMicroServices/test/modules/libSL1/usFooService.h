/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef USFOOSERVICE_H
#define USFOOSERVICE_H

#include <usServiceInterface.h>

namespace us {

struct FooService
{
  virtual ~FooService() {}
  virtual void foo() = 0;
};

}

#endif // USFOOSERVICE_H
