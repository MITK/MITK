/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkToolManagerProvider.h"

// micro service includes
#include <usGetModuleContext.h>
#include <usServiceReference.h>

const char* const mitk::ToolManagerProvider::SEGMENTATION = "";

mitk::ToolManagerProvider::ToolManagerProvider()
{
  m_ToolManagers[SEGMENTATION] = ToolManager::New(nullptr);
}

mitk::ToolManagerProvider::~ToolManagerProvider()
{
}

mitk::ToolManager *mitk::ToolManagerProvider::GetToolManager(const std::string& context)
{
  if (m_ToolManagers.end() == m_ToolManagers.find(context))
    m_ToolManagers[context] = ToolManager::New(nullptr);

  return m_ToolManagers[context].GetPointer();
}

mitk::ToolManagerProvider::ProviderMapType mitk::ToolManagerProvider::GetToolManagers() const
{
  return m_ToolManagers;
}

mitk::ToolManagerProvider *mitk::ToolManagerProvider::GetInstance()
{
  static us::ServiceReference<mitk::ToolManagerProvider> serviceRef;
  static us::ModuleContext *context = us::GetModuleContext();
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
    return nullptr;
  }
}
