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

//  #include "mitkKinectDeviceFactory.h"
//  #include "mitkToFCameraMESASR4000DeviceFactory.h"
//  #include "mitkToFCameraPMDCamCubeDeviceFactory.h"
//  #include "mitkToFCameraPMDRawDataCamCubeDeviceFactory.h"
//  #include "mitkToFCameraPMDPlayerDeviceFactory.h"
//  #include "mitkToFCameraPMDO3DeviceFactory.h"
//  #include "mitkToFCameraPMDCamBoardDeviceFactory.h"
//  #include "mitkToFCameraPMDRawDataCamBoardDeviceFactory.h"
#include "mitkToFCameraMITKPlayerDeviceFactory.h"

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

//.dll do not get deleted automaticaly, so one has to either delete them manualy or just call them if device is actual activated

 // // If kinect activated through CMake, please Load the .dll! else do nothing
 //if (MITK_USE_OF_KINECT)
 //{
 //       LoadLibrary(TEXT("mitkKinectModule.dll"));
 //}

//Loading mitkPMDCamCubeModule.dll of PMDCamCubeModule
      //LoadLibrary(TEXT("mitkPMDCamCubeModule.dll"));



//Loading MesaSR4000Module.dll of the MesaModule
      //LoadLibrary(TEXT("MesaSR4000Module.dll"));


        //Implementing MITKPlayerDevice
        ToFCameraMITKPlayerDeviceFactory* toFCameraMITKPlayerDeviceFactory = new ToFCameraMITKPlayerDeviceFactory();
        ServiceProperties mitkPlayerFactoryProps;
        mitkPlayerFactoryProps["ToFFactoryName"] = toFCameraMITKPlayerDeviceFactory->GetFactoryName();
        context->RegisterService<IToFDeviceFactory>(toFCameraMITKPlayerDeviceFactory, mitkPlayerFactoryProps);
        toFCameraMITKPlayerDeviceFactory->ConnectToFDevice();

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
