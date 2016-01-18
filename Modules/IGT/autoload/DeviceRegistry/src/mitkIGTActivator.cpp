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

#include "mitkIGTActivator.h"

//All Tracking devices, which should be available by default
#include "mitkNDIAuroraTypeInformation.h"
#include "mitkNDIPolarisTypeInformation.h"
#include "mitkVirtualTrackerTypeInformation.h"
#include "mitkMicronTrackerTypeInformation.h"
#include "mitkNPOptitrackTrackingTypeInformation.h"
#include "mitkOpenIGTLinkTypeInformation.h"

namespace mitk
{

  IGTActivator::IGTActivator()
  {
  }

  IGTActivator::~IGTActivator()
  {
  }

  void IGTActivator::Load(us::ModuleContext*)
  {
    m_DeviceTypeCollection.RegisterTrackingDeviceType(new mitk::NDIAuroraTypeInformation());
    m_DeviceTypeCollection.RegisterTrackingDeviceType(new mitk::NDIPolarisTypeInformation());
    m_DeviceTypeCollection.RegisterTrackingDeviceType(new mitk::MicronTrackerTypeInformation());
    m_DeviceTypeCollection.RegisterTrackingDeviceType(new mitk::NPOptitrackTrackingTypeInformation());
    m_DeviceTypeCollection.RegisterTrackingDeviceType(new mitk::VirtualTrackerTypeInformation());
    m_DeviceTypeCollection.RegisterTrackingDeviceType(new mitk::OpenIGTLinkTypeInformation());
    m_DeviceTypeCollection.RegisterAsMicroservice();
  }

  void IGTActivator::Unload(us::ModuleContext*)
  {
    try
    {
      m_DeviceTypeCollection.UnRegisterMicroservice();
    }
    catch (std::exception& e)
    {
      MITK_WARN << "Unable to unregister IGT DeviceTypeCollection Microservice: "<<e.what();
    }
  }

}

US_EXPORT_MODULE_ACTIVATOR(mitk::IGTActivator)
