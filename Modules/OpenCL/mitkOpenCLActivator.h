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
class US_ABI_LOCAL OpenCLActivator : public mitk::ModuleActivator
{
private:

  std::auto_ptr<OclResourceServiceImpl> m_ResourceService;

  /** Static variable for fast internal access to the ResourceService */
  static OclResourceServiceImpl* m_InternalResourceReference;

public:
  /** @brief Load module context */
  void Load(mitk::ModuleContext *context);

  /** @brief Unload module context */
  void Unload(mitk::ModuleContext* );

  static OclResourceService* GetResourceServiceRef();

};



#endif // __mitkOpenCLActivator_h
