/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkOpenCLActivator.h"

void OpenCLActivator::Load(us::ModuleContext *context)
{
  // generate context
  m_ResourceService.reset(new OclResourceServiceImpl);
  us::ServiceProperties props;

  context->RegisterService<OclResourceService>(m_ResourceService.get(), props);
}

void OpenCLActivator::Unload(us::ModuleContext *)
{
  m_ResourceService.release();
}

US_EXPORT_MODULE_ACTIVATOR(OpenCLActivator )
