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
#include "mitkToFDeviceFactoryManager.h"
#include "mitkAbstractToFDeviceFactory.h"

//Microservices
#include <usGetModuleContext.h>


namespace mitk
{
  ToFDeviceFactoryManager::ToFDeviceFactoryManager()
  {
    ModuleContext* context = GetModuleContext();
    m_RegisteredFactoryRefs = context->GetServiceReferences<IToFDeviceFactory>();
    if (m_RegisteredFactoryRefs.empty())
    {
      MITK_ERROR << "No factories registered!";
    }
  }

  ToFDeviceFactoryManager::~ToFDeviceFactoryManager()
  {
  }

  std::vector<std::string> ToFDeviceFactoryManager::GetRegisteredDeviceFactories()
  {
    ModuleContext* context = GetModuleContext();
    //    std::string filter("(" + mitk::ServiceConstants::OBJECTCLASS() + "=" + "org.mitk.services.IToFDeviceFactory)");
    //    std::list<ServiceReference> serviceRef = context->GetServiceReference<IToFDeviceFactory>(/*filter*/);
    std::list<ServiceReference> serviceRefs = context->GetServiceReferences<IToFDeviceFactory>(/*filter*/);
    if (serviceRefs.size() > 0)
    {
      for(std::list<ServiceReference>::iterator it = serviceRefs.begin(); it != serviceRefs.end(); ++it)
      {
        IToFDeviceFactory* service = context->GetService<IToFDeviceFactory>( *it );
        if(service)
        {
          m_RegisteredFactoryNames.push_back(std::string(service->GetFactoryName()));
        }
      }
    }
    return m_RegisteredFactoryNames;
  }

  std::vector<std::string> ToFDeviceFactoryManager::GetConnectedDevices()
  {
    ModuleContext* context = GetModuleContext();
    std::vector<std::string> result;
    std::list<ServiceReference> serviceRefs = context->GetServiceReferences<ToFCameraDevice>();
    if (serviceRefs.size() > 0)
    {
      for(std::list<ServiceReference>::iterator it = serviceRefs.begin(); it != serviceRefs.end(); ++it)
      {
        ToFCameraDevice* service = context->GetService<ToFCameraDevice>( *it );
        if(service)
        {
          result.push_back(std::string(service->GetNameOfClass()));
        }
      }
    }
    if(result.size() == 0)
    {
      MITK_ERROR << "No devices connected!";
    }
    return result;
  }

  ToFCameraDevice* ToFDeviceFactoryManager::GetInstanceOfDevice(int index)
  {
    ModuleContext* context = GetModuleContext();

    std::list<ServiceReference> serviceRefs = context->GetServiceReferences<IToFDeviceFactory>(/*filter*/);
    if (serviceRefs.size() > 0)
    {
      int i = 0;
      for(std::list<ServiceReference>::iterator it = serviceRefs.begin(); it != serviceRefs.end(); ++it)
      {
        IToFDeviceFactory* service = context->GetService<IToFDeviceFactory>( *it );
        if(service && (i == index))
        {
          return dynamic_cast<mitk::AbstractToFDeviceFactory*>(service)->ConnectToFDevice();
        }
        i++;
      }
    }
    MITK_ERROR << "No device generated!";
    return NULL;
  }
}
