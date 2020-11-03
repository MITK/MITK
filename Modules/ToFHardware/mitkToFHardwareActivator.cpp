/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef __mitkToFHardwareModuleActivator_h
#define __mitkToFHardwareModuleActivator_h

// Microservices
#include <usServiceRegistration.h>
#include <usModuleActivator.h>
#include <usModuleContext.h>

#include "mitkIToFDeviceFactory.h"

#include "mitkToFConfig.h"
#include "mitkToFCameraMITKPlayerDeviceFactory.h"

/*
  * This is the module activator for the "ToFHardware" module. It registers services
  * like the IToFDeviceFactory.
*/

namespace mitk
{
class ToFHardwareActivator : public us::ModuleActivator {
public:

    void Load(us::ModuleContext* context) override
    {
        //Registering MITKPlayerDevice as MicroService
        ToFCameraMITKPlayerDeviceFactory* toFCameraMITKPlayerDeviceFactory = new ToFCameraMITKPlayerDeviceFactory();
        us::ServiceProperties mitkPlayerFactoryProps;
        mitkPlayerFactoryProps["ToFFactoryName"] = toFCameraMITKPlayerDeviceFactory->GetFactoryName();
        context->RegisterService<IToFDeviceFactory>(toFCameraMITKPlayerDeviceFactory, mitkPlayerFactoryProps);
        //Create an instance of the player
        toFCameraMITKPlayerDeviceFactory->ConnectToFDevice();

        m_Factories.push_back( toFCameraMITKPlayerDeviceFactory );
    }

    void Unload(us::ModuleContext* ) override
    {
    }

    ~ToFHardwareActivator() override
    {
        if(m_Factories.size() > 0)
        {
            for(std::list< IToFDeviceFactory* >::iterator it = m_Factories.begin(); it != m_Factories.end(); ++it)
            {
                delete (*it);
            }
        }
    }

private:

    std::list< IToFDeviceFactory* > m_Factories;

};
}
US_EXPORT_MODULE_ACTIVATOR(mitk::ToFHardwareActivator)
#endif
