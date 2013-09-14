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

#include "mitkToolManagerProvider.h"

//micro service includes
#include <usServiceReference.h>
#include <usGetModuleContext.h>



mitk::ToolManagerProvider::ToolManagerProvider():
  m_ToolManager(mitk::ToolManager::New(NULL))
{
}


mitk::ToolManagerProvider::~ToolManagerProvider()
{
  this->m_ToolManager = NULL;
}


mitk::ToolManager* mitk::ToolManagerProvider::GetToolManager()
{
  return this->m_ToolManager.GetPointer();
}


mitk::ToolManagerProvider* mitk::ToolManagerProvider::GetInstance()
{
  static us::ServiceReference<mitk::ToolManagerProvider> serviceRef;
  static us::ModuleContext* context = us::GetModuleContext();
  if (!serviceRef)
  {
    // This is either the first time GetInstance() was called,
    // or a SingletonOneService instance has not yet been registered.
    serviceRef = context->GetServiceReference<mitk::ToolManagerProvider>();
  }
  if (serviceRef)
  {
    // We have a valid service reference. It always points to the service
    // with the lowest id (usually the one which was registered first).
    // This still might return a null pointer, if all SingletonOneService
    // instances have been unregistered (during unloading of the library,
    // for example).
    return context->GetService<mitk::ToolManagerProvider>(serviceRef);
  }
  else
  {
    // No SingletonOneService instance was registered yet.
    return 0;
  }
}
