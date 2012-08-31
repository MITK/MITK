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
#ifndef __mitkToFHardwareModuleActivator_h
#define __mitkToFHardwareModuleActivator_h

// Microservices
#include <usServiceRegistration.h>
#include <usModuleActivator.h>
#include "mitkModuleContext.h"

#include <mitkModuleActivator.h>
#include "mitkIToFDeviceFactory.h"

#include "mitkToFConfig.h"
#include "mitkKinectDeviceFactory.h"
#include "mitkToFCameraMESADeviceFactory.h"
#include "mitkToFCameraMESASR4000DeviceFactory.h"
#include "mitkToFPlayerDeviceFactory.h"
#include "mitkToFPMDCamBoardDeviceFactory.h"
#include "mitkToFPMDCamCubeDeviceFactory.h"
#include "mitkToFPMDO3DeviceFactory.h"
#include "mitkToFPMDPlayerDeviceFactory.h"
#include "mitkToFPMDRawDataCamCubeDeviceFactory.h"
#include "mitkToFPMDRawDataCamBoardDeviceFactory.h"


/*
  * This is the module activator for the "ToFHardware" module. It registers services
  * like the IToFDeviceFactory.
  */

namespace mitk
{
class ToFHardwareActivator : public mitk::ModuleActivator {
public:

    void Load(mitk::ModuleContext* context)
    {
        //Einbinden + factories registrieren

        std::string string(MITK_TOF_CAMERAS);
        //       string.replace(";"," ");
        std::cout << "cameras " << string << std::endl;

        //Implementing KinectDeiveFactory
        KinectDeviceFactory* kinectFactory = new KinectDeviceFactory();
        ServiceProperties kinectFactoryProps;
        kinectFactoryProps["ToFFactoryName"] = kinectFactory->GetFactoryName();
        context->RegisterService<IToFDeviceFactory>(kinectFactory, kinectFactoryProps);

        //Implementing MESADeviceFactory
        ToFCameraMESADeviceFactory* toFCameraMESADeviceFactory = new ToFCameraMESADeviceFactory();
        ServiceProperties mitkMESADeviceFactoryProps;
        mitkMESADeviceFactoryProps["ToFFactoryName"] = toFCameraMESADeviceFactory->GetFactoryName();
        context->RegisterService<IToFDeviceFactory>(toFCameraMESADeviceFactory, mitkMESADeviceFactoryProps);

        //Implementing MESASR4000DeviceFactory
        ToFCameraMESASR4000DeviceFactory* toFCameraMESASR4000DeviceFactory = new ToFCameraMESASR4000DeviceFactory();
        ServiceProperties mitkMESASR4000FactoryProps;
        mitkMESASR4000FactoryProps["ToFFactoryName"] = toFCameraMESASR4000DeviceFactory->GetFactoryName();
        context->RegisterService<IToFDeviceFactory>(toFCameraMESASR4000DeviceFactory, mitkMESASR4000FactoryProps);

        //Implementing MITKPlayerDevice
        ToFPlayerDeviceFactory* toFPlayerDeviceFactory = new ToFPlayerDeviceFactory();
        ServiceProperties mitkPlayerFactoryProps;
        mitkPlayerFactoryProps["ToFFactoryName"] = toFPlayerDeviceFactory->GetFactoryName();
        context->RegisterService<IToFDeviceFactory>(toFPlayerDeviceFactory, mitkPlayerFactoryProps);

        //Implementing CamBoardDeviceFactory
        ToFPMDCamBoardDeviceFactory* toFPMDCamBoardDeviceFactory = new ToFPMDCamBoardDeviceFactory();
        ServiceProperties camBoardFactoryProps;
        camBoardFactoryProps["ToFFactoryName"] = toFPMDCamBoardDeviceFactory->GetFactoryName();
        context->RegisterService<IToFDeviceFactory>(toFPMDCamBoardDeviceFactory, camBoardFactoryProps);

        //Implementing CamCubeDeviceFactory
        ToFPMDCamCubeDeviceFactory* toFPMDCamCubeDeviceFactory = new ToFPMDCamCubeDeviceFactory();
        ServiceProperties camCubeFactoryProps;
        camCubeFactoryProps["ToFFactoryName"] = toFPMDCamCubeDeviceFactory->GetFactoryName();
        context->RegisterService<IToFDeviceFactory>(toFPMDCamCubeDeviceFactory,camCubeFactoryProps);

        //Implementing PMD O3D DeviceFactory
        ToFPMDO3DeviceFactory* toFPMDO3DeviceFactory = new ToFPMDO3DeviceFactory();
        ServiceProperties o3FactoryProps;
        o3FactoryProps["ToFFactoryName"] = toFPMDO3DeviceFactory->GetFactoryName();
        context->RegisterService<IToFDeviceFactory>(toFPMDO3DeviceFactory,o3FactoryProps);

        //Implementing PMD Player DeviceFactory
        ToFPMDPlayerDeviceFactory* toFPMDPlayerDeviceFactory = new ToFPMDPlayerDeviceFactory();
        ServiceProperties pMDPlayerFactoryProps;
        pMDPlayerFactoryProps["ToFFactoryName"] = toFPMDPlayerDeviceFactory->GetFactoryName();
        context->RegisterService<IToFDeviceFactory>(toFPMDPlayerDeviceFactory,pMDPlayerFactoryProps);

        //Implementing PMD Raw Data Cam Cube DeviceFactory
        ToFPMDRawDataCamCubeDeviceFactory* toFPMDRawDataCamCubeDeviceFactory = new ToFPMDRawDataCamCubeDeviceFactory();
        ServiceProperties rawCamCubeFactoryProps;
        rawCamCubeFactoryProps["ToFFactoryName"] = toFPMDRawDataCamCubeDeviceFactory->GetFactoryName();
        context->RegisterService<IToFDeviceFactory>(toFPMDRawDataCamCubeDeviceFactory,rawCamCubeFactoryProps);

        //Implementing PMD Raw Data Cam Board DeviceFactory
        ToFPMDRawDataCamBoardDeviceFactory* toFPMDRawDataCamBoardDeviceFactory = new ToFPMDRawDataCamBoardDeviceFactory();
        ServiceProperties rawCamBoardFactoryProps;
        rawCamBoardFactoryProps["ToFFactoryName"] = toFPMDRawDataCamBoardDeviceFactory->GetFactoryName();
        context->RegisterService<IToFDeviceFactory>(toFPMDRawDataCamBoardDeviceFactory,rawCamBoardFactoryProps);

        m_Factories.push_back( kinectFactory );
    }

    void Unload(mitk::ModuleContext* )
    {
    }

    ~ToFHardwareActivator()
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
US_EXPORT_MODULE_ACTIVATOR(mitkToFHardware, mitk::ToFHardwareActivator)
#endif
