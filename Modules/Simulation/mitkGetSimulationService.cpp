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

#include "mitkGetSimulationService.h"
#include <mitkISimulationService.h>
#include <usModuleContext.h>
#include <usServiceReference.h>

mitk::ISimulationService* mitk::GetSimulationService(us::ModuleContext* moduleContext)
{
  if (moduleContext == NULL)
    return NULL;

  us::ServiceReference<mitk::ISimulationService> serviceReference = moduleContext->GetServiceReference<mitk::ISimulationService>();
  return moduleContext->GetService<mitk::ISimulationService>(serviceReference);
}
