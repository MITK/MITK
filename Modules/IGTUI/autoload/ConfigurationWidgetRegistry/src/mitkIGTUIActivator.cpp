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