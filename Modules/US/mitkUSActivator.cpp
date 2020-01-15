/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkUSActivator.h"
#include "mitkUSDevicePersistence.h"

mitk::USActivator::USActivator() : m_Context(nullptr)
{
}

mitk::USActivator::~USActivator()
{
}

void mitk::USActivator::Load(us::ModuleContext* context)
{
  m_Context = context;

  // load us video devices from the harddisk
  mitk::USDevicePersistence::Pointer devicePersistence = mitk::USDevicePersistence::New();
  m_Devices = devicePersistence->RestoreLastDevices();

  // register all devices in microservice
  for (std::vector<mitk::USDevice::Pointer>::iterator it = m_Devices.begin();
    it != m_Devices.end(); ++it)
  {
    (*it)->Initialize();
  }

  // to be notified about every register and unregister of an USDevice
  context->AddServiceListener(this, &mitk::USActivator::OnServiceEvent,
    "(" + us::ServiceConstants::OBJECTCLASS() + "=" + us_service_interface_iid<mitk::USDevice>() + ")");
}

void mitk::USActivator::Unload(us::ModuleContext* context)
{
  m_Context = context;

  // no notifiation of the following unregistering is wanted
  context->RemoveServiceListener(this, &mitk::USActivator::OnServiceEvent);

  // store us video devices on the harddisk
  mitk::USDevicePersistence::Pointer devicePersistence = mitk::USDevicePersistence::New();
  devicePersistence->StoreCurrentDevices();

  // mark all devices as no longer needed (they will be unregistered in their destructor)
  m_Devices.erase(m_Devices.begin(), m_Devices.end());
}

void mitk::USActivator::OnServiceEvent(const us::ServiceEvent event)
{
  if ( ! m_Context )
  {
    MITK_WARN("us::ModuleActivator")("USActivator")
        << "OnServiceEvent listener called without having a module context in "
        << "the activator. Cannot handle event.";
    return;
  }

  us::ServiceReference<mitk::USDevice> service = event.GetServiceReference();
  mitk::USDevice::Pointer device = m_Context->GetService(us::ServiceReference<mitk::USDevice>(service));

  switch (event.GetType())
  {
    case us::ServiceEvent::REGISTERED:
      // hold new device in vector
      m_Devices.push_back(device.GetPointer());
      break;
    case us::ServiceEvent::UNREGISTERING:
    {
      // unregistered device does not need to be hold any longer
      std::vector<mitk::USDevice::Pointer>::iterator it = find(m_Devices.begin(), m_Devices.end(), device.GetPointer());
      if (it != m_Devices.end()) { m_Devices.erase(it); }
      break;
    }
    default:
      MITK_DEBUG("us::ModuleActivator")("USActivator")
          << "Received uninteresting service event: " << event.GetType();
      break;
  }
}
