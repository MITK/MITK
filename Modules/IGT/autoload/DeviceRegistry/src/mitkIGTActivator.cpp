/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkIGTConfig.h"
#include "mitkIGTActivator.h"

//All Tracking devices, which should be available by default
#include "mitkNDIAuroraTypeInformation.h"
#include "mitkNDIPolarisTypeInformation.h"
#include "mitkVirtualTrackerTypeInformation.h"
#ifdef MITK_USE_MICRON_TRACKER
#include "mitkMicronTrackerTypeInformation.h"
#endif
#ifdef MITK_USE_OPTITRACK_TRACKER
#include "mitkNPOptitrackTrackingTypeInformation.h"
#endif
#include "mitkOpenIGTLinkTypeInformation.h"
#ifdef MITK_USE_POLHEMUS_TRACKER
#include "mitkPolhemusTrackerTypeInformation.h"
#endif

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
    m_DeviceTypeCollection.RegisterTrackingDeviceType(new mitk::VirtualTrackerTypeInformation());
    m_DeviceTypeCollection.RegisterTrackingDeviceType(new mitk::OpenIGTLinkTypeInformation());
#ifdef MITK_USE_OPTITRACK_TRACKER
    m_DeviceTypeCollection.RegisterTrackingDeviceType(new mitk::NPOptitrackTrackingTypeInformation());
#endif
#ifdef MITK_USE_MICRON_TRACKER
    m_DeviceTypeCollection.RegisterTrackingDeviceType(new mitk::MicronTrackerTypeInformation());
#endif
#ifdef MITK_USE_POLHEMUS_TRACKER
    m_DeviceTypeCollection.RegisterTrackingDeviceType(new mitk::PolhemusTrackerTypeInformation());
#endif
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
