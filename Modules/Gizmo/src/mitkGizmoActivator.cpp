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

#include "mitkGizmoActivator.h"

// Micro Services
#include <usGetModuleContext.h>
#include <usModuleInitialization.h>
#include <usModuleActivator.h>
#include <usModuleContext.h>
#include <usModuleSettings.h>
#include <usModuleEvent.h>
#include <usModule.h>
#include <usModuleResource.h>
#include <usModuleResourceStream.h>
#include <usServiceTracker.h>

void GizmoActivator::Load(us::ModuleContext* context)
{
  this->m_Context = context;
}

void GizmoActivator::Unload(us::ModuleContext* )
{
}

US_EXPORT_MODULE_ACTIVATOR(GizmoActivator)
