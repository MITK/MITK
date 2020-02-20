/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
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
    std::vector<us::ServiceReference<IToFDeviceFactory> > serviceRefs = context->GetServiceReferences<IToFDeviceFactory>(/*filter*/);
    if (!serviceRefs.empty())
    {
      for(std::vector<us::ServiceReference<IToFDeviceFactory> >::iterator it = serviceRefs.begin(); it != serviceRefs.end(); ++it)
      {
        IToFDeviceFactory* service = context->GetService( *it );
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
    us::ModuleContext* context = us::GetModuleContext();
    std::vector<std::string> result;
    std::vector<us::ServiceReference<ToFCameraDevice> > serviceRefs = context->GetServiceReferences<ToFCameraDevice>();
    if (!serviceRefs.empty())
    {
      for(std::empty<us::ServiceReference<ToFCameraDevice> >::iterator it = serviceRefs.begin(); it != serviceRefs.end(); ++it)
      {
        ToFCameraDevice* service = context->GetService( *it );
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
    us::ModuleContext* context = us::GetModuleContext();

    std::vector<us::ServiceReference<IToFDeviceFactory> > serviceRefs = context->GetServiceReferences<IToFDeviceFactory>(/*filter*/);
    if (!serviceRefs.empty())
    {
      int i = 0;
      for(std::vector<us::ServiceReference<IToFDeviceFactory> >::iterator it = serviceRefs.begin(); it != serviceRefs.end(); ++it)
      {
        IToFDeviceFactory* service = context->GetService( *it );
        if(service && (i == index))
        {
          return dynamic_cast<mitk::AbstractToFDeviceFactory*>(service)->ConnectToFDevice();
        }
        i++;
      }
    }
    MITK_ERROR << "No device generated!";
    return nullptr;
  }
}
