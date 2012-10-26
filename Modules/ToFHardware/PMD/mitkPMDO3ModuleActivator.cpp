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
#ifndef __mitkPMDO3ModuleActivator_h
#define __mitkPMDO3ModuleActivator_h

// Microservices
#include <usServiceRegistration.h>
#include <usModuleActivator.h>
#include "mitkModuleContext.h"

#include <mitkModuleActivator.h>
#include "mitkIToFDeviceFactory.h"

#include "mitkToFConfig.h"
#include "mitkToFCameraPMDPlayerDeviceFactory.h"

#include "mitkToFCameraPMDO3DeviceFactory.h"


//Implementing a Device on start
#include "mitkAbstractToFDeviceFactory.h"


/*
  * This is the module activator for the "mitkPMDCamO3Module" module. It registers services
  * like the IToFDeviceFactory.
  */

namespace mitk
{
class PMDO3ModuleActivator : public ModuleActivator {
public:

    void Load(mitk::ModuleContext* context)
    {


        //Implementing PMD O3D DeviceFactory
        ToFCameraPMDO3DeviceFactory* toFCameraPMDO3DeviceFactory = new ToFCameraPMDO3DeviceFactory();
        ServiceProperties o3FactoryProps;
        o3FactoryProps["ToFFactoryName"] = toFCameraPMDO3DeviceFactory->GetFactoryName();
        context->RegisterService<IToFDeviceFactory>(toFCameraPMDO3DeviceFactory,o3FactoryProps);

        toFCameraPMDO3DeviceFactory->ConnectToFDevice();

           m_Factories.push_back(toFCameraPMDO3DeviceFactory);

    }

    void Unload(mitk::ModuleContext* )
    {
    }

    ~PMDO3ModuleActivator()
    {
        //todo iterieren über liste m_Factories und löschen
        if(m_Factories.size() > 0)
        {
            for(std::list< IToFDeviceFactory* >::iterator it = m_Factories.begin(); it != m_Factories.end(); ++it)
            {
                delete (*it); //todo wie genau löschen?
            }
        }
    }

private:

    std::list< IToFDeviceFactory* > m_Factories;

};
}
US_EXPORT_MODULE_ACTIVATOR(mitkPMDO3Module, mitk::PMDO3ModuleActivator)
#endif
