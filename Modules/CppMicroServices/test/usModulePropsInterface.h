/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef USMODULEPROPSINTERFACE_H
#define USMODULEPROPSINTERFACE_H

#include <usServiceInterface.h>
#include <usServiceProperties.h>

namespace us {

struct ModulePropsInterface
{
  typedef ServiceProperties Properties;

  virtual ~ModulePropsInterface() {}

  virtual const Properties& GetProperties() const = 0;
};

}

#endif // USMODULEPROPSINTERFACE_H
