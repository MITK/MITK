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
//--------------------------------------------------------------------------------------------------------------------------------------------
//.dll do not get updated(deleted) automatically. One has to either to delete them manualy or just call them if a device is activated

     //If Kinect is activated through CMake, please load the .dll by uncommenting the next line!
          LoadLibrary(TEXT("mitkKinectModule.dll"));


    //If MesaSR4000Module is activated through CMake, please load the MesaSR4000Module.dll by uncommenting the next line
          //LoadLibrary(TEXT("MesaSR4000Module.dll"));


     //If CamBoard is activated through CMake, please load the .dll by uncommenting the next line!
          LoadLibrary(TEXT("mitkPMDCamBoardModule.dll"));


     //If CamCube is activated through CMake, please load the .dll by uncommenting the next line!
          LoadLibrary(TEXT("mitkPMDCamCubeModule.dll"));


     //If PMDO3D is activated through CMake, please load the .dll by uncommenting the next line!
          LoadLibrary(TEXT("mitkPMDO3Module.dll"));

//--------------------------------------------------------------------------------------------------------------------------------------------


        //Implementing MITKPlayerDevice
        ToFCameraMITKPlayerDeviceFactory* toFCameraMITKPlayerDeviceFactory = new ToFCameraMITKPlayerDeviceFactory();
        ServiceProperties mitkPlayerFactoryProps;
        mitkPlayerFactoryProps["ToFFactoryName"] = toFCameraMITKPlayerDeviceFactory->GetFactoryName();
        context->RegisterService<IToFDeviceFactory>(toFCameraMITKPlayerDeviceFactory, mitkPlayerFactoryProps);
        toFCameraMITKPlayerDeviceFactory->ConnectToFDevice();

        m_Factories.push_back( toFCameraMITKPlayerDeviceFactory );
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
