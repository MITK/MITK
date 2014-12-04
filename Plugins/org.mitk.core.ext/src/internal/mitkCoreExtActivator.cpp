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

#include "mitkCoreExtActivator.h"

#include "mitkCoreExtConstants.h"

#include "mitkInputDeviceRegistry.h"

#include <berryPlatform.h>
#include <berryIPreferencesService.h>

#include <QtPlugin>

namespace mitk
{
  void CoreExtActivator::start(ctkPluginContext* context)
  {
    Q_UNUSED(context)

    this->StartInputDeviceModules();
  }

  void CoreExtActivator::stop(ctkPluginContext* context)
  {
    Q_UNUSED(context)
  }

  void mitk::CoreExtActivator::StartInputDeviceModules()
  {
    IInputDeviceRegistry::Pointer inputDeviceRegistry(new mitk::InputDeviceRegistry());
    berry::Platform::GetServiceRegistry().RegisterService(
          mitk::CoreExtConstants::INPUTDEVICE_SERVICE,
          inputDeviceRegistry);

    // Gets the last setting of the preferences; if a device was selected,
    // it will still be activated after a restart
    berry::IPreferencesService::Pointer prefService = berry::Platform::GetServiceRegistry()
        .GetServiceById<berry::IPreferencesService>(berry::IPreferencesService::ID);
    berry::IPreferences::Pointer extPreferencesNode =
        prefService->GetSystemPreferences()->Node(CoreExtConstants::INPUTDEVICE_PREFERENCES);

    // Initializes the modules
    std::vector<IInputDeviceDescriptor::Pointer> descriptors(inputDeviceRegistry->GetInputDevices());
    for (std::vector<IInputDeviceDescriptor::Pointer>::const_iterator it = descriptors.begin();
         it != descriptors.end(); ++it)
    {
      if (extPreferencesNode->GetBool((*it)->GetID(), false))
      {
        IInputDevice::Pointer temp = (*it)->CreateInputDevice();
        temp->RegisterInputDevice();
      }
    }
  } // end method StartInputDeviceModules
} // end namespace mitk

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
  Q_EXPORT_PLUGIN2(org_mitk_core_ext, mitk::CoreExtActivator)
#endif
