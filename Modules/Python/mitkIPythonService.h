/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef mitkIPythonService_h
#define mitkIPythonService_h
#define PY_SSIZE_T_CLEAN

#include <mitkServiceInterface.h>
#include <MitkPythonExports.h>
#include "mitkCommon.h"

namespace mitk
{

  class MITKPYTHON_EXPORT IPythonService
  {
  public:
    IPythonService();      
    ~IPythonService();
  };
}
MITK_DECLARE_SERVICE_INTERFACE(mitk::IPythonService, "org.mitk.services.IPythonService")

#endif
