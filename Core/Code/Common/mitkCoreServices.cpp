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

#include "mitkCoreServices.h"

#include "mitkIShaderRepository.h"

#include "mitkGetModuleContext.h"
#include "mitkModuleContext.h"
#include "mitkServiceReference.h"

namespace mitk {

IShaderRepository* CoreServices::GetShaderRepository()
{
  IShaderRepository* shaderRepo = NULL;
  ServiceReference serviceRef = GetModuleContext()->GetServiceReference<IShaderRepository>();
  if (serviceRef)
  {
    shaderRepo = GetModuleContext()->GetService<IShaderRepository>(serviceRef);
  }
  return shaderRepo;
}

}
