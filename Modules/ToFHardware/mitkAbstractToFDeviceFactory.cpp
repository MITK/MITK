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
#include "mitkAbstractToFDeviceFactory.h"
#include <mitkCameraIntrinsics.h>
#include <mitkCameraIntrinsicsProperty.h>
//Microservices
#include <usModuleContext.h>
#include <usGetModuleContext.h>
#include <usModule.h>
#include <usModuleResource.h>
#include <usModuleResourceStream.h>

#include <tinyxml.h>

mitk::ToFCameraDevice::Pointer mitk::AbstractToFDeviceFactory::ConnectToFDevice()
{
  ToFCameraDevice::Pointer device = CreateToFCameraDevice();
  mitk::CameraIntrinsics::Pointer cameraIntrinsics = GetCameraIntrinsics();
  device->SetProperty("CameraIntrinsics", mitk::CameraIntrinsicsProperty::New(cameraIntrinsics));
  m_Devices.push_back(device);

  us::ModuleContext* context = us::GetModuleContext();
  us::ServiceProperties deviceProps;
  deviceProps["ToFDeviceName"] = GetCurrentDeviceName();
  m_DeviceRegistrations.insert(std::make_pair(device.GetPointer(), context->RegisterService(device.GetPointer(),deviceProps)));
  return device;
}

void mitk::AbstractToFDeviceFactory::DisconnectToFDevice(const ToFCameraDevice::Pointer& device)
{
  std::map<ToFCameraDevice*,us::ServiceRegistration<ToFCameraDevice> >::iterator i = m_DeviceRegistrations.find(device.GetPointer());
  if (i == m_DeviceRegistrations.end()) return;

  i->second.Unregister();
  m_DeviceRegistrations.erase(i);

  m_Devices.erase(std::remove(m_Devices.begin(), m_Devices.end(), device), m_Devices.end());
}

size_t mitk::AbstractToFDeviceFactory::GetNumberOfDevices()
{
  return m_Devices.size();
}

mitk::CameraIntrinsics::Pointer mitk::AbstractToFDeviceFactory::GetCameraIntrinsics()
{
  us::ModuleResource resource = GetIntrinsicsResource();
  if (! resource.IsValid())
  {
    MITK_WARN << "Could not load resource '" << resource.GetName() << "'. CameraIntrinsics are invalid!";
  }

  // Create ResourceStream from Resource
  us::ModuleResourceStream resStream(resource);

  // Parse XML
  TiXmlDocument xmlDocument;
  resStream >> xmlDocument;

  //Retrieve Child Element and convert to CamerIntrinsics
  TiXmlElement* element = xmlDocument.FirstChildElement();
  mitk::CameraIntrinsics::Pointer intrinsics = mitk::CameraIntrinsics::New();
  intrinsics->FromXML(element);

  return intrinsics;
}

us::ModuleResource mitk::AbstractToFDeviceFactory::GetIntrinsicsResource()
{
  us::Module* module = us::GetModuleContext()->GetModule();
  return module->GetResource("CalibrationFiles/Default_Parameters.xml");
  MITK_WARN << "Loaded Default CameraIntrinsics. Overwrite AbstractToFDeviceFactory::GetIntrinsicsResource() if you want to define your own.";
}
