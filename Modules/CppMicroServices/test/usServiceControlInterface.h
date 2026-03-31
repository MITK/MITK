/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef USSERVICECONTROLINTERFACE_H
#define USSERVICECONTROLINTERFACE_H

#include <usGlobalConfig.h>
#include <usServiceInterface.h>

#include <string>

namespace us {

struct ServiceControlInterface
{

  virtual ~ServiceControlInterface() {}

  virtual void ServiceControl(int service, const std::string& operation, int ranking) = 0;
};

}

#endif // USSERVICECONTROLINTERFACE_H
