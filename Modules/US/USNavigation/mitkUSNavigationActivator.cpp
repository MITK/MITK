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

#include "mitkUSNavigationActivator.h"

mitk::USNavigationActivator::USNavigationActivator() : m_Context(0)
{
}

mitk::USNavigationActivator::~USNavigationActivator()
{
}

void mitk::USNavigationActivator::Load(us::ModuleContext* context)
{
  m_Context = context;

  // to be notified about every register and unregister of an USCombinedModality
  context->AddServiceListener(this, &mitk::USNavigationActivator::OnServiceEvent,
    "(" + us::ServiceConstants::OBJECTCLASS() + "=" + us_service_interface_iid<mitk::USDevice>() + ")");
}

void mitk::USNavigationActivator::Unload(us::ModuleContext* context)
{
  m_Context = context;

  // no notifiation of the following unregistering is wanted
  context->RemoveServiceListener(this, &mitk::USNavigationActivator::OnServiceEvent);

  // mark all devices as no longer needed (they will be unregistered in their destructor)
  m_Devices.erase(m_Devices.begin(), m_Devices.end());
}

void mitk::USNavigationActivator::OnServiceEvent(const us::ServiceEvent event)
{
  if ( ! m_Context )
  {
    MITK_WARN("us::ModuleActivator")("USNavigationActivator")
        << "OnServiceEvent listener called without having a module context in "
        << "the activator. Cannot handle event.";
    return;
  }

  // just USVideoDevice objects need processing in this method
  us::ServiceReference<mitk::USDevice> service = event.GetServiceReference();
  if ( service.GetProperty(mitk::USDevice::US_PROPKEY_CLASS).ToString() != "org.mitk.modules.us.USCombinedModality" )
  {
    MITK_INFO("us::ModuleActivator")("USNavigationActivator")
        << "Device is no USCombinedModality.";
    return;
  }

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
      MITK_DEBUG("us::ModuleActivator")("USNavigationActivator")
          << "Received uninteresting service event: " << event.GetType();
      break;
  }
}
