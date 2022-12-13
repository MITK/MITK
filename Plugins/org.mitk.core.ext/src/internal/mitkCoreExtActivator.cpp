/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkCoreExtActivator.h"

#include "mitkCoreExtConstants.h"
#include "mitkLogMacros.h"

#include <ctkPluginContext.h>

#include <mitkCoreServices.h>
#include <mitkIPreferencesService.h>
#include <mitkIPreferences.h>

#include <usModuleInitialization.h>

US_INITIALIZE_MODULE

namespace mitk
{
  void CoreExtActivator::start(ctkPluginContext* context)
  {
    Q_UNUSED(context)

    this->StartInputDeviceModules(context);
  }

  void CoreExtActivator::stop(ctkPluginContext* context)
  {
    Q_UNUSED(context)
  }

  void CoreExtActivator::StartInputDeviceModules(ctkPluginContext* context)
  {
    m_InputDeviceRegistry.reset(new InputDeviceRegistry());
    context->registerService<mitk::IInputDeviceRegistry>(m_InputDeviceRegistry.data());

    auto* prefService = mitk::CoreServices::GetPreferencesService();
    auto* extPreferencesNode = prefService->GetSystemPreferences()->Node(CoreExtConstants::INPUTDEVICE_PREFERENCES.toStdString());

    // Initializes the modules
    QList<IInputDeviceDescriptor::Pointer> descriptors(m_InputDeviceRegistry->GetInputDevices());
    for (QList<IInputDeviceDescriptor::Pointer>::const_iterator it = descriptors.begin();
         it != descriptors.end(); ++it)
    {
      if (extPreferencesNode->GetBool((*it)->GetID().toStdString(), false))
      {
        IInputDevice::Pointer temp = (*it)->CreateInputDevice();
        temp->RegisterInputDevice();
      }
    }
  }

  CoreExtActivator::~CoreExtActivator()
  {
  }

} // end namespace mitk
