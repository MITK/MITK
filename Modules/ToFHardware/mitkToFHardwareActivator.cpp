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
//#include "mitkKinectDeviceFactory.h"
#include "mitkToFCameraMESASR4000DeviceFactory.h"
#include "mitkToFCameraPMDPlayerDeviceFactory.h"
#include "mitkToFCameraPMDCamBoardDeviceFactory.h"
#include "mitkToFCameraPMDCamCubeDeviceFactory.h"
#include "mitkToFCameraMITKPlayerDeviceFactory.h"
#include "mitkToFCameraPMDO3DeviceFactory.h"
#include "mitkToFCameraPMDRawDataCamCubeDeviceFactory.h"
#include "mitkToFCameraPMDRawDataCamBoardDeviceFactory.h"


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
        //KinectDeviceFactory* kinectFactory = new KinectDeviceFactory();
        //ServiceProperties kinectFactoryProps;
        //kinectFactoryProps["ToFFactoryName"] = kinectFactory->GetFactoryName();
        //context->RegisterService<IToFDeviceFactory>(kinectFactory, kinectFactoryProps);
        LoadLibrary(TEXT("mitkKinectModule.dll"));

        //Implementing MESASR4000DeviceFactory
        ToFCameraMESASR4000DeviceFactory* toFCameraMESASR4000DeviceFactory = new ToFCameraMESASR4000DeviceFactory();
        ServiceProperties mitkMESASR4000FactoryProps;
        mitkMESASR4000FactoryProps["ToFFactoryName"] = toFCameraMESASR4000DeviceFactory->GetFactoryName();
        context->RegisterService<IToFDeviceFactory>(toFCameraMESASR4000DeviceFactory, mitkMESASR4000FactoryProps);

        //Implementing MITKPlayerDevice
        ToFCameraMITKPlayerDeviceFactory* toFCameraMITKPlayerDeviceFactory = new ToFCameraMITKPlayerDeviceFactory();
        ServiceProperties mitkPlayerFactoryProps;
        mitkPlayerFactoryProps["ToFFactoryName"] = toFCameraMITKPlayerDeviceFactory->GetFactoryName();
        context->RegisterService<IToFDeviceFactory>(toFCameraMITKPlayerDeviceFactory, mitkPlayerFactoryProps);

        //Implementing CamBoardDeviceFactory
        ToFCameraPMDCamBoardDeviceFactory* toFCameraPMDCamBoardDeviceFactory = new ToFCameraPMDCamBoardDeviceFactory();
        ServiceProperties camBoardFactoryProps;
        camBoardFactoryProps["ToFFactoryName"] = toFCameraPMDCamBoardDeviceFactory->GetFactoryName();
        context->RegisterService<IToFDeviceFactory>(toFCameraPMDCamBoardDeviceFactory, camBoardFactoryProps);

        //Implementing CamCubeDeviceFactory
        ToFCameraPMDCamCubeDeviceFactory* toFCameraPMDCamCubeDeviceFactory = new ToFCameraPMDCamCubeDeviceFactory();
        ServiceProperties camCubeFactoryProps;
        camCubeFactoryProps["ToFFactoryName"] = toFCameraPMDCamCubeDeviceFactory->GetFactoryName();
        context->RegisterService<IToFDeviceFactory>(toFCameraPMDCamCubeDeviceFactory,camCubeFactoryProps);

        //Implementing PMD O3D DeviceFactory
        ToFCameraPMDO3DeviceFactory* toFCameraPMDO3DeviceFactory = new ToFCameraPMDO3DeviceFactory();
        ServiceProperties o3FactoryProps;
        o3FactoryProps["ToFFactoryName"] = toFCameraPMDO3DeviceFactory->GetFactoryName();
        context->RegisterService<IToFDeviceFactory>(toFCameraPMDO3DeviceFactory,o3FactoryProps);

        //Implementing PMD Player DeviceFactory
        ToFCameraPMDPlayerDeviceFactory* toFCameraPMDPlayerDeviceFactory = new ToFCameraPMDPlayerDeviceFactory();
        ServiceProperties pMDPlayerFactoryProps;
        pMDPlayerFactoryProps["ToFFactoryName"] = toFCameraPMDPlayerDeviceFactory->GetFactoryName();
        context->RegisterService<IToFDeviceFactory>(toFCameraPMDPlayerDeviceFactory,pMDPlayerFactoryProps);

        //Implementing PMD Raw Data Cam Cube DeviceFactory
        ToFCameraPMDRawDataCamCubeDeviceFactory* toFCameraPMDRawDataCamCubeDeviceFactory = new ToFCameraPMDRawDataCamCubeDeviceFactory();
        ServiceProperties rawCamCubeFactoryProps;
        rawCamCubeFactoryProps["ToFFactoryName"] = toFCameraPMDRawDataCamCubeDeviceFactory->GetFactoryName();
        context->RegisterService<IToFDeviceFactory>(toFCameraPMDRawDataCamCubeDeviceFactory,rawCamCubeFactoryProps);

        //Implementing PMD Raw Data Cam Board DeviceFactory
        ToFCameraPMDRawDataCamBoardDeviceFactory* toFCameraPMDRawDataCamBoardDeviceFactory = new ToFCameraPMDRawDataCamBoardDeviceFactory();
        ServiceProperties rawCamBoardFactoryProps;
        rawCamBoardFactoryProps["ToFFactoryName"] = toFCameraPMDRawDataCamBoardDeviceFactory->GetFactoryName();
        context->RegisterService<IToFDeviceFactory>(toFCameraPMDRawDataCamBoardDeviceFactory,rawCamBoardFactoryProps);

        //m_Factories.push_back( kinectFactory );
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
