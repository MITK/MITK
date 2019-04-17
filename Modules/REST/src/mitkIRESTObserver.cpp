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
