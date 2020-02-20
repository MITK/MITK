/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkIRESTObserver.h>
#include <mitkIRESTManager.h>

#include <usModuleContext.h>
#include <usGetModuleContext.h>

mitk::IRESTObserver::~IRESTObserver()
{
  auto context = us::GetModuleContext();
  auto managerRef = context->GetServiceReference<IRESTManager>();
  if (managerRef)
  {
    auto manager = context->GetService(managerRef);
    if (manager)
      manager->HandleDeleteObserver(this);
  }
}
