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
#ifndef __mitkToFCameraPMDCamCubeActivator_h
#define __mitkToFCameraPMDCamCubeActivator_h

// Microservices
#include <usServiceRegistration.h>
#include <usModuleActivator.h>
#include "mitkModuleContext.h"

#include <mitkModuleActivator.h>
#include "mitkIToFDeviceFactory.h"

#include "mitkToFConfig.h"
#include "mitkToFCameraPMDCamCubeDeviceFactory.h"
#include "mitkToFCameraPMDRawDataCamCubeDeviceFactory.h"

#include "mitkToFCameraPMDPlayerDeviceFactory.h"

//Implementing a Device on start
#include "mitkAbstractToFDeviceFactory.h"


/*
  * This is the module activator for the "mitkPMDCamCubeModule" module. It registers services
  * like the IToFDeviceFactory.
  */

namespace mitk
{
class PMDCamCubeModuleActivator : public ModuleActivator {
public:

    void Load(mitk::ModuleContext* context)
    {
        //Implementing CamCubeDeviceFactory

      MITK_INFO << "Modul ToFCameraPMDCamCubeActivator ";
        ToFCameraPMDCamCubeDeviceFactory* toFCameraPMDCamCubeDeviceFactory = new ToFCameraPMDCamCubeDeviceFactory();
        ServiceProperties camCubeFactoryProps;
        camCubeFactoryProps["ToFFactoryName"] = toFCameraPMDCamCubeDeviceFactory->GetFactoryName();
        context->RegisterService<IToFDeviceFactory>(toFCameraPMDCamCubeDeviceFactory,camCubeFactoryProps);

        toFCameraPMDCamCubeDeviceFactory->ConnectToFDevice();


        ////Implementing PMD Raw Data Cam Cube DeviceFactory
        //ToFCameraPMDRawDataCamCubeDeviceFactory* toFCameraPMDRawDataCamCubeDeviceFactory = new ToFCameraPMDRawDataCamCubeDeviceFactory();
        //ServiceProperties rawCamCubeFactoryProps;
        //rawCamCubeFactoryProps["ToFFactoryName"] = toFCameraPMDRawDataCamCubeDeviceFactory->GetFactoryName();
        //context->RegisterService<IToFDeviceFactory>(toFCameraPMDRawDataCamCubeDeviceFactory,rawCamCubeFactoryProps);

        //toFCameraPMDRawDataCamCubeDeviceFactory->ConnectToFDevice();


        m_Factories.push_back(toFCameraPMDCamCubeDeviceFactory);

    }

    void Unload(mitk::ModuleContext* )
    {
    }

    ~PMDCamCubeModuleActivator()
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
US_EXPORT_MODULE_ACTIVATOR(mitkPMDCamCubeModule, mitk::PMDCamCubeModuleActivator)
#endif
