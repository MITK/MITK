/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkRESTServiceActivator.h"
#include <usModuleContext.h>

void MitkRESTServiceActivator::Load(us::ModuleContext *context)
{
  m_RESTManager.reset(new mitk::RESTManager);
  context->RegisterService<mitk::IRESTManager>(m_RESTManager.get());
}

void MitkRESTServiceActivator::Unload(us::ModuleContext *)
{
}

US_EXPORT_MODULE_ACTIVATOR(MitkRESTServiceActivator)
