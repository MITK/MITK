/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkIGTConfig.h"
#include "mitkIGTUIActivator.h"

//All Tracking devices, which should be available by default
#include "mitkNDIAuroraTypeInformation.h"
#include "mitkNDIPolarisTypeInformation.h"
#include "mitkVirtualTrackerTypeInformation.h"
#include "mitkMicronTrackerTypeInformation.h"
#include "mitkNPOptitrackTrackingTypeInformation.h"
#include "mitkOpenIGTLinkTypeInformation.h"

//standard tracking devices, which always should be avaiable
#include "QmitkNDIAuroraWidget.h"
#include "QmitkNDIPolarisWidget.h"
#include "QmitkMicronTrackerWidget.h"
#include "QmitkNPOptitrackWidget.h"
#include "QmitkVirtualTrackerWidget.h"
#include "QmitkOpenIGTLinkWidget.h"

// Polhemus tracking device
#ifdef MITK_USE_POLHEMUS_TRACKER
#include "mitkPolhemusTrackerTypeInformation.h"
#include "QmitkPolhemusTrackerWidget.h"
#endif

namespace mitk
{
  IGTUIActivator::IGTUIActivator()
  {
  }

  IGTUIActivator::~IGTUIActivator()
  {
  }

  void IGTUIActivator::Load(us::ModuleContext*)
  {
    m_DeviceWidgetCollection.RegisterAsMicroservice();
    //Add widgets of standard tracking devices
    m_DeviceWidgetCollection.RegisterTrackingDeviceWidget(mitk::NDIAuroraTypeInformation::GetTrackingDeviceName(), new QmitkNDIAuroraWidget);
    m_DeviceWidgetCollection.RegisterTrackingDeviceWidget(mitk::NDIPolarisTypeInformation::GetTrackingDeviceName(), new QmitkNDIPolarisWidget);
    m_DeviceWidgetCollection.RegisterTrackingDeviceWidget(mitk::MicronTrackerTypeInformation::GetTrackingDeviceName(), new QmitkMicronTrackerWidget);
    m_DeviceWidgetCollection.RegisterTrackingDeviceWidget(mitk::NPOptitrackTrackingTypeInformation::GetTrackingDeviceName(), new QmitkNPOptitrackWidget);
    m_DeviceWidgetCollection.RegisterTrackingDeviceWidget(mitk::VirtualTrackerTypeInformation::GetTrackingDeviceName(), new QmitkVirtualTrackerWidget);
    m_DeviceWidgetCollection.RegisterTrackingDeviceWidget(mitk::OpenIGTLinkTypeInformation::GetTrackingDeviceName(), new QmitkOpenIGTLinkWidget);
#ifdef MITK_USE_POLHEMUS_TRACKER
    m_DeviceWidgetCollection.RegisterTrackingDeviceWidget(mitk::PolhemusTrackerTypeInformation::GetTrackingDeviceName(), new QmitkPolhemusTrackerWidget);
#endif
  }

  void IGTUIActivator::Unload(us::ModuleContext*)
  {
    try
    {
      m_DeviceWidgetCollection.UnRegisterMicroservice();
    }
    catch (std::exception& e)
    {
      MITK_WARN << "Unable to unregister IGTUI DeviceWidgetCollection Microservice: " << e.what();
    }
  }
}

US_EXPORT_MODULE_ACTIVATOR(mitk::IGTUIActivator)
