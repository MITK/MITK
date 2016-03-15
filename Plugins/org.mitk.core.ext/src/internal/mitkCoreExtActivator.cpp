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
#include "mitkLogMacros.h"

#include <berryPlatform.h>
#include <berryIPreferences.h>
#include <berryIPreferencesService.h>

#include <ctkPluginContext.h>

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

    // Gets the last setting of the preferences; if a device was selected,
    // it will still be activated after a restart
    ctkServiceReference prefServiceRef = context->getServiceReference<berry::IPreferencesService>();
    if (!prefServiceRef)
    {
      MITK_WARN << "Preferences service not available";
      return;
    }
    berry::IPreferencesService* prefService = context->getService<berry::IPreferencesService>(prefServiceRef);
    berry::IPreferences::Pointer extPreferencesNode =
        prefService->GetSystemPreferences()->Node(CoreExtConstants::INPUTDEVICE_PREFERENCES);

    // Initializes the modules
    QList<IInputDeviceDescriptor::Pointer> descriptors(m_InputDeviceRegistry->GetInputDevices());
    for (QList<IInputDeviceDescriptor::Pointer>::const_iterator it = descriptors.begin();
         it != descriptors.end(); ++it)
    {
      if (extPreferencesNode->GetBool((*it)->GetID(), false))
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
