/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkROIIOModuleActivator.h"

#include <mitkROIIOMimeTypes.h>
#include "mitkROIIO.h"

#include <usModuleContext.h>

US_EXPORT_MODULE_ACTIVATOR(mitk::ROIIOModuleActivator)

void mitk::ROIIOModuleActivator::Load(us::ModuleContext* context)
{
  auto mimeTypes = MitkROIIOMimeTypes::Get();

  us::ServiceProperties props;
  props[us::ServiceConstants::SERVICE_RANKING()] = 10;

  for (auto mimeType : mimeTypes)
    context->RegisterService(mimeType, props);

  m_FileIOs.push_back(std::make_shared<ROIIO>());
}

void mitk::ROIIOModuleActivator::Unload(us::ModuleContext*)
{
}
