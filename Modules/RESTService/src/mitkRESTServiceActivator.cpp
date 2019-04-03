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
