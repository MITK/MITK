/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef __mitkOpenCLActivator_h
#define __mitkOpenCLActivator_h

#include "mitkOclResourceServiceImpl_p.h"

#include <usModuleActivator.h>
#include <usModuleContext.h>
#include <usGetModuleContext.h>
#include <usServiceProperties.h>

#include <set>
#include <algorithm>
#include <memory>

/**
 * @class OpenCLActivator
 *
 * @brief Custom activator for the OpenCL Module in order to register
 * and provide the OclResourceService
 */
class US_ABI_LOCAL OpenCLActivator : public us::ModuleActivator
{
private:

  std::unique_ptr<OclResourceServiceImpl> m_ResourceService;

public:
  /** @brief Load module context */
  void Load(us::ModuleContext *context);

  /** @brief Unload module context */
  void Unload(us::ModuleContext* );

};



#endif // __mitkOpenCLActivator_h
