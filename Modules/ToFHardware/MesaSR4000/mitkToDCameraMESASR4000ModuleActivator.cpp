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
#ifndef __mitkMESASR4000Activator_h
#define __mitkMESASR4000Activator_h

// Microservices
#include <usServiceRegistration.h>
#include <usModuleActivator.h>
#include "mitkModuleContext.h"

#include <mitkModuleActivator.h>
#include "mitkIToFDeviceFactory.h"

#include "mitkToFConfig.h"
#include "mitkToFCameraMESASR4000DeviceFactory.h"



/*
  * This is the module activator for the "mitkPMDCamCubeModule" module. It registers services
  * like the IToFDeviceFactory.
  */

namespace mitk
{
class MESASR4000ModuleActivator : public mitk::ModuleActivator {
public:

    void Load(mitk::ModuleContext* context)
    {
      //Implementing MESASR4000DeviceFactory
        ToFCameraMESASR4000DeviceFactory* toFCameraMESASR4000DeviceFactory = new ToFCameraMESASR4000DeviceFactory();
        ServiceProperties mitkMESASR4000FactoryProps;
        mitkMESASR4000FactoryProps["ToFFactoryName"] = toFCameraMESASR4000DeviceFactory->GetFactoryName();
        context->RegisterService<IToFDeviceFactory>(toFCameraMESASR4000DeviceFactory, mitkMESASR4000FactoryProps);

        toFCameraMESASR4000DeviceFactory->ConnectToFDevice();

        m_Factories.push_back(toFCameraMESASR4000DeviceFactory);
    }

    void Unload(mitk::ModuleContext* )
    {
    }

    ~MESASR4000ModuleActivator()
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
US_EXPORT_MODULE_ACTIVATOR(mitkMESASR4000Module, mitk::MESASR4000ModuleActivator)
#endif
