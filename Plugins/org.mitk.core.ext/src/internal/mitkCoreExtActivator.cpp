/*=========================================================================

Program:   BlueBerry Platform
Language:  C++
Date:      $Date$
Version:   $Revision: 17020 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkCoreExtActivator.h"
#include "mitkPicFileReader.h"

#include <mitkCoreExtObjectFactory.h>
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

    // see bug 10816
    // don't let the linker optimize the dependency away
    mitk::PicFileReader::New();

    RegisterCoreExtObjectFactory();
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

Q_EXPORT_PLUGIN2(org_mitk_core_ext, mitk::CoreExtActivator)
